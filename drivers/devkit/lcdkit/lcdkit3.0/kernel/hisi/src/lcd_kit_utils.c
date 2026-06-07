/*
 * lcd_kit_utils.c
 *
 * lcdkit utils function for lcd driver
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

#include "lcd_kit_utils.h"
#include <linux/hisi/hw_cmdline_parse.h> // for runmode_is_factory
#include "global_ddr_map.h"
#include "hisi_fb.h"
#include "lcd_kit_disp.h"
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

static void lcd_kit_vesa3_75x_dual(struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}
	lcd_kit_vesa3_config(pinfo);
}


void lcd_kit_compress_config(int mode, struct hisi_panel_info *pinfo)
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
		lcd_kit_vesa3x_single(pinfo);
		break;
	case IFBC_TYPE_VESA3_75X_DUAL:
		lcd_kit_vesa3_75x_dual(pinfo);
		break;
	case IFBC_TYPE_VESA3X_DUAL:
		lcd_kit_vesa3x_dual(pinfo);
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
	struct dsi_cmd_desc lcd_reg_cmd;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = (struct hisi_fb_data_type *)pdata;
	lcd_reg_cmd.dtype = cmds->dtype;
	lcd_reg_cmd.vc = cmds->vc;
	lcd_reg_cmd.wait = cmds->wait;
	lcd_reg_cmd.waittype = cmds->waittype;
	lcd_reg_cmd.dlen = cmds->dlen;
	lcd_reg_cmd.payload = cmds->payload;
	ret = mipi_dsi_lread_reg(out, &lcd_reg_cmd, len, hisifd->mipi_dsi0_base);
	if (ret) {
		LCD_KIT_INFO("read error, ret=%d\n", ret);
		return ret;
	}
	return ret;
}

#define PROJECTID_LEN 9
#define PROJECTID_PRD_LEN 4
static int lcd_kit_check_project_id(void)
{
	int i = 0;

	for (; i < PROJECTID_PRD_LEN; i++) {
		if (isalnum((disp_info->project_id.id)[i]) == 0)
			return LCD_KIT_FAIL;
	}
	for (; i < PROJECTID_LEN; i++) {
		if (isdigit((disp_info->project_id.id)[i]) == 0)
			return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_read_project_id(void)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	if (disp_info->project_id.support == 0)
		return LCD_KIT_OK;

	memset(disp_info->project_id.id, 0, sizeof(disp_info->project_id.id));
	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_kit_read_project_id)
		return panel_ops->lcd_kit_read_project_id();

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_dsi_cmds_rx(hisifd, (uint8_t *)disp_info->project_id.id,
		&disp_info->project_id.cmds);
	if (ret == LCD_KIT_OK && lcd_kit_check_project_id() == LCD_KIT_OK) {
		LCD_KIT_INFO("project id is %s\n", disp_info->project_id.id);
		return LCD_KIT_OK;
	}
	if (disp_info->project_id.default_project_id) {
		strncpy(disp_info->project_id.id, disp_info->project_id.default_project_id, PROJECTID_LEN + 1);
		LCD_KIT_ERR("use default project id:%s\n", disp_info->project_id.default_project_id);
	}
	return LCD_KIT_FAIL;
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

uint8_t g_last_fps_scence = LCD_FPS_SCENCE_NORMAL;
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
	if (pinfo->fps_scence == g_last_fps_scence) {
		LCD_KIT_DEBUG("scence is same and needn't send fps cmds\n");
		return ret;
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

int lcd_kit_updt_fps_scence(struct platform_device *pdev, uint32_t scence)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (!pdev) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
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
		mutex_lock(&disp_info->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_STOP;
		mutex_lock(&disp_info->mipi_lock);
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.off_cmds);
		break;
	case ALPM_ON_MIDDLE_LIGHT:
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.high_light_cmds);
		mutex_lock(&disp_info->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_RUNNING;
		mutex_lock(&disp_info->mipi_lock);
		break;
	case ALPM_EXIT:
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.exit_cmds);
		mutex_lock(&disp_info->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_RUNNING;
		mutex_lock(&disp_info->mipi_lock);
		hisifd->aod_mode = 0;
		break;
	case ALPM_ON_LOW_LIGHT:
		ret = lcd_kit_dsi_cmds_tx(hisifd, &disp_info->alpm.low_light_cmds);
		mutex_lock(&disp_info->mipi_lock);
		if (common_info->esd.support)
			common_info->esd.status = ESD_RUNNING;
		mutex_lock(&disp_info->mipi_lock);
		break;
	default:
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
		down(&disp_info->lcd_kit_sem);
		if (bl_level == 0 && last_bl_level != 0)
			lcd_kit_charger_ctrl(LCD_KIT_BL, 0);
		else if (last_bl_level == 0 && bl_level != 0)
			lcd_kit_charger_ctrl(LCD_KIT_BL, 1);
		last_bl_level = bl_level;
		up(&disp_info->lcd_kit_sem);
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
		break;
	case EN_DISPLAY_REGION_B:
		disp_info->dbv_stat.last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[SLAVE_REGION] = curr_time;
		break;
	case EN_DISPLAY_REGION_AB:
	case EN_DISPLAY_REGION_AB_FOLDED:
		disp_info->dbv_stat.last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.last_time[FOLD_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[SLAVE_REGION] = curr_time;
		disp_info->dbv_stat.pwon_last_time[FOLD_REGION] = curr_time;
		break;
	default:
		break;
	}
}

static void _lcd_set_dbv_stat(int region, unsigned int level)
{
	uint32_t delta;
	struct timeval curr_time, tmp;

	do_gettimeofday(&curr_time);
	switch (region) {
	case PRIMARY_REGION:
		tmp = disp_info->dbv_stat.last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.dbv[PRIMARY_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.last_time[PRIMARY_REGION]);
		break;
	case SLAVE_REGION:
		tmp = disp_info->dbv_stat.last_time[SLAVE_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.dbv[SLAVE_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.last_time[SLAVE_REGION]);
		break;
	case FOLD_REGION:
		tmp = disp_info->dbv_stat.last_time[FOLD_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.dbv[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.last_time[FOLD_REGION]);
		break;
	case REGION_MAX:
		tmp = disp_info->dbv_stat.last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.dbv[PRIMARY_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.last_time[SLAVE_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.dbv[SLAVE_REGION] += (level * delta);
		tmp = disp_info->dbv_stat.last_time[FOLD_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.dbv[FOLD_REGION] += (level * delta);
		do_gettimeofday(&disp_info->dbv_stat.last_time[PRIMARY_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.last_time[SLAVE_REGION]);
		do_gettimeofday(&disp_info->dbv_stat.last_time[FOLD_REGION]);
		break;
	default:
		break;
	}
}

static void _lcd_set_pwon_stat(int region)
{
	uint32_t delta;
	struct timeval curr_time, tmp;

	do_gettimeofday(&curr_time);
	switch (region) {
	case PRIMARY_REGION:
		tmp = disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[PRIMARY_REGION] += delta;
		break;
	case SLAVE_REGION:
		tmp = disp_info->dbv_stat.pwon_last_time[SLAVE_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[SLAVE_REGION] += delta;
		break;
	case FOLD_REGION:
		tmp = disp_info->dbv_stat.pwon_last_time[FOLD_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[FOLD_REGION] += delta;
		break;
	case REGION_MAX:
		tmp = disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION];
		/* change s to ms */
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[PRIMARY_REGION] += delta;
		tmp = disp_info->dbv_stat.pwon_last_time[SLAVE_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[SLAVE_REGION] += delta;
		tmp = disp_info->dbv_stat.pwon_last_time[FOLD_REGION];
		delta = (curr_time.tv_sec - tmp.tv_sec) * 1000 +
			(curr_time.tv_usec - tmp.tv_usec) / 1000;
		disp_info->dbv_stat.pwon[FOLD_REGION] += delta;
		break;
	default:
		break;
	}
}

static void lcd_set_dbv_stat(struct hisi_fb_data_type *hisifd,
	uint32_t level)
{
	static uint32_t last_level;
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
	_lcd_set_dbv_stat(region, last_level);
	last_level = level;
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[PRIMARY_REGION]:%d\n",
		disp_info->dbv_stat.dbv[PRIMARY_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[SLAVE_REGION]:%d\n",
		disp_info->dbv_stat.dbv[SLAVE_REGION]);
	LCD_KIT_INFO("disp_info->dbv_stat.dbv[FOLD_REGION]:%d\n",
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
	_lcd_set_dbv_stat(region, hisifd->bl_level);
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
	_lcd_set_pwon_stat(region);
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

void lcd_switch_region(struct hisi_fb_data_type *hisifd,
	uint8_t curr_mode, uint8_t pre_mode)
{
	struct timeval tmp;

	if (!disp_info->dbv_stat.support)
		return;
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null!\n");
		return;
	}
	if ((curr_mode == EN_DISPLAY_REGION_A) &&
		(pre_mode == EN_DISPLAY_REGION_B)) {
		do_gettimeofday(&tmp);
		disp_info->dbv_stat.last_time[PRIMARY_REGION] = tmp;
		disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION] = tmp;
		_lcd_set_dbv_stat(SLAVE_REGION, hisifd->bl_level);
		_lcd_set_pwon_stat(SLAVE_REGION);
	} else if ((curr_mode == EN_DISPLAY_REGION_A) &&
		((pre_mode == EN_DISPLAY_REGION_AB) ||
		(pre_mode == EN_DISPLAY_REGION_AB_FOLDED))) {
		_lcd_set_dbv_stat(SLAVE_REGION, hisifd->bl_level);
		_lcd_set_dbv_stat(FOLD_REGION, hisifd->bl_level);
		_lcd_set_pwon_stat(SLAVE_REGION);
		_lcd_set_pwon_stat(FOLD_REGION);
	} else if ((curr_mode == EN_DISPLAY_REGION_B) &&
		(pre_mode == EN_DISPLAY_REGION_A)) {
		do_gettimeofday(&tmp);
		disp_info->dbv_stat.last_time[SLAVE_REGION] = tmp;
		disp_info->dbv_stat.pwon_last_time[SLAVE_REGION] = tmp;
		_lcd_set_dbv_stat(PRIMARY_REGION, hisifd->bl_level);
		_lcd_set_pwon_stat(PRIMARY_REGION);
	} else if ((curr_mode == EN_DISPLAY_REGION_B) &&
		((pre_mode == EN_DISPLAY_REGION_AB) ||
		(pre_mode == EN_DISPLAY_REGION_AB_FOLDED))) {
		_lcd_set_dbv_stat(PRIMARY_REGION, hisifd->bl_level);
		_lcd_set_dbv_stat(FOLD_REGION, hisifd->bl_level);
		_lcd_set_pwon_stat(PRIMARY_REGION);
		_lcd_set_pwon_stat(FOLD_REGION);
	} else if (((curr_mode == EN_DISPLAY_REGION_AB) ||
		(curr_mode == EN_DISPLAY_REGION_AB_FOLDED)) &&
		(pre_mode == EN_DISPLAY_REGION_A)) {
		do_gettimeofday(&tmp);
		disp_info->dbv_stat.last_time[SLAVE_REGION] = tmp;
		disp_info->dbv_stat.pwon_last_time[SLAVE_REGION] = tmp;
		do_gettimeofday(&tmp);
		disp_info->dbv_stat.last_time[FOLD_REGION] = tmp;
		disp_info->dbv_stat.pwon_last_time[FOLD_REGION] = tmp;
	} else if (((curr_mode == EN_DISPLAY_REGION_AB) ||
		(curr_mode == EN_DISPLAY_REGION_AB_FOLDED)) &&
		(pre_mode == EN_DISPLAY_REGION_B)) {
		do_gettimeofday(&tmp);
		disp_info->dbv_stat.last_time[PRIMARY_REGION] = tmp;
		disp_info->dbv_stat.pwon_last_time[PRIMARY_REGION] = tmp;
		do_gettimeofday(&tmp);
		disp_info->dbv_stat.last_time[FOLD_REGION] = tmp;
		disp_info->dbv_stat.pwon_last_time[FOLD_REGION] = tmp;
	}
}

int lcd_get_dbv_stat(struct hisi_fb_data_type *hisifd,
	uint32_t *dbv_stat, uint32_t dbv_len,
	uint32_t *pwon_stat, uint32_t pwon_len)
{
	struct hisi_panel_info *pinfo = NULL;

	if (!disp_info->dbv_stat.support)
		return LCD_KIT_OK;
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!dbv_stat) {
		LCD_KIT_ERR("dbv_stat is null!\n");
		return LCD_KIT_FAIL;
	}
	if (!pwon_stat) {
		LCD_KIT_ERR("pwon_stat is null!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null!\n");
		return LCD_KIT_FAIL;
	}
	_lcd_get_dbv_stat(hisifd, dbv_stat, dbv_len);
	_lcd_get_pwon_stat(hisifd, pwon_stat, pwon_len);
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
		mutex_lock(&common_info->hbm.hbm_lock);
		ret = common_ops->set_mipi_backlight(hisifd, bl_level);
		pinfo->hbm_blcode_ts = ktime_get();
		mutex_unlock(&common_info->hbm.hbm_lock);
	} else {
		ret = common_ops->set_mipi_backlight(hisifd, bl_level);
	}
	lcd_set_dbv_stat(hisifd, bl_level);
	if (power_hdl->lcd_backlight.buf[POWER_MODE] == REGULATOR_MODE) {
		/* enable/disable backlight */
		down(&disp_info->lcd_kit_sem);
		if (bl_level == 0 && last_bl_level != 0) {
			lcd_kit_charger_ctrl(LCD_KIT_BL, 0);
		} else if (last_bl_level == 0 && bl_level != 0) {
			lcd_kit_charger_ctrl(LCD_KIT_BL, 1);
			LCD_KIT_INFO("bl_level = %d!\n", bl_level);
		}
		last_bl_level = bl_level;
		up(&disp_info->lcd_kit_sem);
	}
	return ret;
}

int lcd_kit_dsi_fifo_is_full(const char __iomem *dsi_base)
{
	unsigned long dw_jiffies;
	uint32_t pkg_status, phy_status;
	int is_timeout = 1;

	/* read status register */
	dw_jiffies = jiffies + HZ;
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status & 0x2) != 0x2 && !(phy_status & 0x2)) {
			is_timeout = 0;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

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
	unsigned long dw_jiffies;
	uint32_t pkg_status, phy_status;
	int is_timeout = 1;

	/* read status register */
	dw_jiffies = jiffies + HZ;
	do {
		pkg_status = inp32(dsi_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
		phy_status = inp32(dsi_base + MIPIDSI_PHY_STATUS_OFFSET);
		if ((pkg_status & 0x1) == 0x1 && !(phy_status & 0x2)) {
			is_timeout = 0;
			break;
		}
	} while (time_after(dw_jiffies, jiffies));

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
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,vesa-slice-width",
		&pinfo->vesa_dsc.slice_width, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,vesa-slice-height",
		&pinfo->vesa_dsc.slice_height, 0);
}

void lcd_kit_pinfo_init(struct device_node *np, struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return;
	}

	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-xres",
		&pinfo->xres, 1440); // 1440 is default value
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-yres",
		&pinfo->yres, 2560); // 2560 is default value
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-width",
		&pinfo->width, 73); // 73 is default value
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-height",
		&pinfo->height, 130); // 130 is default value
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-orientation",
		&pinfo->orientation, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bpp", &pinfo->bpp, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bgr-fmt",
		&pinfo->bgr_fmt, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-type",
		&pinfo->bl_set_type, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-min",
		&pinfo->bl_min, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-max",
		&pinfo->bl_max, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-def",
		&pinfo->bl_default, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-cmd-type",
		&pinfo->type, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,panel-frc-enable",
		&pinfo->frc_enable, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,panel-esd-skip-mipi-check",
		&pinfo->esd_skip_mipi_check, 1);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-ifbc-type",
		&pinfo->ifbc_type, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-ic-ctrl-type",
		&pinfo->bl_ic_ctrl_mode, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,blpwm-div",
		&pinfo->blpwm_out_div_value, 0);
	OF_PROPERTY_READ_U64_RETURN(np, "lcd-kit,panel-pxl-clk",
		&pinfo->pxl_clk_rate);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-pxl-clk-div",
		&pinfo->pxl_clk_rate_div, 1);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-vsyn-ctr-type",
		&pinfo->vsync_ctrl_type, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-xcc-set-in-isr-support",
		&pinfo->xcc_set_in_isr_support, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-bl-pwm-preci-type",
		&pinfo->blpwm_precision_type, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,non-check-ldi-porch",
		&pinfo->non_check_ldi_porch, 0);

	/* effect info */
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,sbl-support",
		&pinfo->sbl_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,gamma-support",
		&pinfo->gamma_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,gmp-support",
		&pinfo->gmp_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,dbv-curve-mapped-support",
		&pinfo->dbv_curve_mapped_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,color-temp-support",
		&pinfo->color_temperature_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,color-temp-rectify-support",
		&pinfo->color_temp_rectify_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,comform-mode-support",
		&pinfo->comform_mode_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,cinema-mode-support",
		&pinfo->cinema_mode_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,xcc-support",
		&pinfo->xcc_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,hiace-support",
		&pinfo->hiace_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,dither-support",
		&pinfo->dither_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,panel-ce-support",
		&pinfo->panel_effect_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,arsr1p-sharpness-support",
		&pinfo->arsr1p_sharpness_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,prefix-sharp-1D-support",
		&pinfo->prefix_sharpness1D_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,prefix-sharp-2D-support",
		&pinfo->prefix_sharpness2D_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,post-xcc-support",
		&pinfo->post_xcc_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,scaling-ratio-threshold",
		&pinfo->scaling_ratio_threshold, 0);
	if (pinfo->hiace_support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,iglobal-hist-black-pos",
			&pinfo->hiace_param.iGlobalHistBlackPos, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,iglobal-hist-white-pos",
			&pinfo->hiace_param.iGlobalHistWhitePos, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,iglobal-hist-black-weight",
			&pinfo->hiace_param.iGlobalHistBlackWeight, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,iglobal-hist-white-weight",
			&pinfo->hiace_param.iGlobalHistWhiteWeight, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,iglobal-hist-zero-cut-ratio",
			&pinfo->hiace_param.iGlobalHistZeroCutRatio, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,iglobal-hist-slope-cut-ratio",
			&pinfo->hiace_param.iGlobalHistSlopeCutRatio, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,imax-lcd-luminance",
			&pinfo->hiace_param.iMaxLcdLuminance, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,imin-lcd-luminance",
			&pinfo->hiace_param.iMinLcdLuminance, 0);
	}
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,color-temp-rectify-r",
		&pinfo->color_temp_rectify_R);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,color-temp-rectify-g",
		&pinfo->color_temp_rectify_G);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,color-temp-rectify-b",
		&pinfo->color_temp_rectify_B);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,prefix-ce-support",
		&pinfo->prefix_ce_support);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,acm-support",
		&pinfo->acm_support);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,acm-ce-support",
		&pinfo->acm_ce_support);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,smart-color-mode-support",
		&pinfo->smart_color_mode_support);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,acm-valid-num",
		&pinfo->acm_valid_num);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh0", &pinfo->r0_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh0", &pinfo->r0_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh1", &pinfo->r1_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh1", &pinfo->r1_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh2", &pinfo->r2_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh2", &pinfo->r2_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh3", &pinfo->r3_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh3", &pinfo->r3_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh4", &pinfo->r4_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh4", &pinfo->r4_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh5", &pinfo->r5_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh5", &pinfo->r5_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_hh6", &pinfo->r6_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_lh6", &pinfo->r6_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh0",
		&pinfo->video_r0_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh0",
		&pinfo->video_r0_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh1",
		&pinfo->video_r1_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh1",
		&pinfo->video_r1_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh2",
		&pinfo->video_r2_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh2",
		&pinfo->video_r2_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh3",
		&pinfo->video_r3_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh3",
		&pinfo->video_r3_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh4",
		&pinfo->video_r4_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh4",
		&pinfo->video_r4_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh5",
		&pinfo->video_r5_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh5",
		&pinfo->video_r5_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_hh6",
		&pinfo->video_r6_hh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,r_video_lh6",
		&pinfo->video_r6_lh);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,mask-delay-time-before-fp",
		&pinfo->mask_delay_time_before_fp);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,mask-delay-time-after-fp",
		&pinfo->mask_delay_time_after_fp);
	OF_PROPERTY_READ_U64_RETURN(np, "lcd-kit,left-time-to-te-us",
		&pinfo->left_time_to_te_us);
	OF_PROPERTY_READ_U64_RETURN(np, "lcd-kit,right-time-to-te-us",
		&pinfo->right_time_to_te_us);
	OF_PROPERTY_READ_U64_RETURN(np, "lcd-kit,te-interval-time-us",
		&pinfo->te_interval_us);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,bl-delay-frame",
		&pinfo->bl_delay_frame);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,fphbm-entry-delay-afterBL",
		&pinfo->hbm_entry_delay, 0);

	/* sbl info */
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-stren-limit",
		&pinfo->smart_bl.strength_limit);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-cal-a",
		&pinfo->smart_bl.calibration_a);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-cal-b",
		&pinfo->smart_bl.calibration_b);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-cal-c",
		&pinfo->smart_bl.calibration_c);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-cal-d",
		&pinfo->smart_bl.calibration_d);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-tf-ctl",
		&pinfo->smart_bl.t_filter_control);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-bl-min",
		&pinfo->smart_bl.backlight_min);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-bl-max",
		&pinfo->smart_bl.backlight_max);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-bl-scale",
		&pinfo->smart_bl.backlight_scale);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-am-light-min",
		&pinfo->smart_bl.ambient_light_min);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-filter-a",
		&pinfo->smart_bl.filter_a);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-filter-b",
		&pinfo->smart_bl.filter_b);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-logo-left",
		&pinfo->smart_bl.logo_left);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-logo-top",
		&pinfo->smart_bl.logo_top);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-variance-intensity-space",
		&pinfo->smart_bl.variance_intensity_space);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-slope-max",
		&pinfo->smart_bl.slope_max);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,sbl-slope-min",
		&pinfo->smart_bl.slope_min);

	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,dpi01-set-change",
		&pinfo->dpi01_exchange_flag);
	/* ldi info */
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,h-back-porch",
		&pinfo->ldi.h_back_porch);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,h-front-porch",
		&pinfo->ldi.h_front_porch);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,h-pulse-width",
		&pinfo->ldi.h_pulse_width);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,v-back-porch",
		&pinfo->ldi.v_back_porch);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,v-front-porch",
		&pinfo->ldi.v_front_porch);
	OF_PROPERTY_READ_U32_RETURN(np, "lcd-kit,v-pulse-width",
		&pinfo->ldi.v_pulse_width);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,ldi-hsync-plr",
		&pinfo->ldi.hsync_plr);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,ldi-vsync-plr",
		&pinfo->ldi.vsync_plr);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,ldi-pixel-clk-plr",
		&pinfo->ldi.pixelclk_plr);
	OF_PROPERTY_READ_U8_RETURN(np, "lcd-kit,ldi-data-en-plr",
		&pinfo->ldi.data_en_plr);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,ldi-dpi0-overlap-size",
		&pinfo->ldi.dpi0_overlap_size, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,ldi-dpi1-overlap-size",
		&pinfo->ldi.dpi1_overlap_size, 0);

	/* mipi info */
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,mipi-lane-nums",
		&pinfo->mipi.lane_nums, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,mipi-color-mode",
		&pinfo->mipi.color_mode, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,mipi-vc",
		&pinfo->mipi.vc, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-burst-mode",
		&pinfo->mipi.burst_mode, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dsi-bit-clk",
		&pinfo->mipi.dsi_bit_clk, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-max-tx-esc-clk",
		&pinfo->mipi.max_tx_esc_clk, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dsi-bit-clk-val1",
		&pinfo->mipi.dsi_bit_clk_val1, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dsi-bit-clk-val2",
		&pinfo->mipi.dsi_bit_clk_val2, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dsi-bit-clk-val3",
		&pinfo->mipi.dsi_bit_clk_val3, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dsi-bit-clk-val4",
		&pinfo->mipi.dsi_bit_clk_val4, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dsi-bit-clk-val5",
		&pinfo->mipi.dsi_bit_clk_val5, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,mipi-non-continue-enable",
		&pinfo->mipi.non_continue_en, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-post-adjust",
		&pinfo->mipi.clk_post_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-pre-adjust",
		&pinfo->mipi.clk_pre_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-t-hs-prepare-adjust",
		&pinfo->mipi.clk_t_hs_prepare_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-t-lpx-adjust",
		&pinfo->mipi.clk_t_lpx_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-data-t-lpx-adjust",
		&pinfo->mipi.data_t_lpx_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-t-hs-trail-adjust",
		&pinfo->mipi.clk_t_hs_trial_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-t-hs-exit-adjust",
		&pinfo->mipi.clk_t_hs_exit_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-clk-t-hs-zero-adjust",
		&pinfo->mipi.clk_t_hs_zero_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-data-t-hs-zero-adjust",
		&pinfo->mipi.data_t_hs_zero_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-data-t-hs-trail-adjust",
		&pinfo->mipi.data_t_hs_trial_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np,
		"lcd-kit,mipi-data-lane-lp2hs-time-adjust",
		&pinfo->mipi.data_lane_lp2hs_time_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np,
		"lcd-kit,mipi-data-t-hs-prepare-adjust",
		&pinfo->mipi.data_t_hs_prepare_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-rg-vrefsel-vcm-adjust",
		&pinfo->mipi.rg_vrefsel_vcm_adjust, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-phy-mode",
		&pinfo->mipi.phy_mode, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-lp11-flag",
		&pinfo->mipi.lp11_flag, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-hs-wr-to-time",
		&pinfo->mipi.hs_wr_to_time, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-phy-update",
		&pinfo->mipi.phy_m_n_count_update, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,mipi-dsi-upt-support",
		&pinfo->dsi_bit_clk_upt_support, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,video-idle-mode-support",
		&pinfo->video_idle_mode, 0);
	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,mipi-dsi-timing-support",
		&pinfo->mipi.dsi_timing_support, 0);

	if (pinfo->mipi.dsi_timing_support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-h-sync-area",
			&pinfo->mipi.hsa, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-h-back-porch",
			&pinfo->mipi.hbp, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-h-line-time",
			&pinfo->mipi.hline_time, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-dpi-h-size",
			&pinfo->mipi.dpi_hsize, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-v-sync-area",
			&pinfo->mipi.vsa, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-v-back-porch",
			&pinfo->mipi.vbp, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,mipi-v-front-porch",
			&pinfo->mipi.vfp, 0);
	}
	/* dirty region update */
	if (common_info->dirty_region.support) {
		pinfo->dirty_region_updt_support =
			common_info->dirty_region.support;
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-left-align",
			&pinfo->dirty_region_info.left_align);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-right-align",
			&pinfo->dirty_region_info.right_align);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-top-align",
			&pinfo->dirty_region_info.top_align);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-bott-align",
			&pinfo->dirty_region_info.bottom_align);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-width-align",
			&pinfo->dirty_region_info.w_align);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-height-align",
			&pinfo->dirty_region_info.h_align);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-width-min",
			&pinfo->dirty_region_info.w_min);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-height-min",
			&pinfo->dirty_region_info.h_min);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-top-start",
			&pinfo->dirty_region_info.top_start);
		OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np,
			"lcd-kit,dirty-bott-start",
			&pinfo->dirty_region_info.bottom_start);
	}
	/* bl pwm */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,blpwm-disable",
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

	/* esd */
	if (common_info->esd.support) {
		pinfo->esd_enable = 1;
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,esd-recovery-max-count",
			&pinfo->esd_recovery_max_count, 10);
		/* esd_check_max_count set 3 times as default */
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,esd-check-max-count",
			&pinfo->esd_check_max_count, 3);
	}
}

int lcd_kit_panel_version_init(struct hisi_fb_data_type *hisifd)
{
	int i, j, ret;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	if (disp_info->panel_version.enter_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->panel_version.enter_cmds);
		if (ret) {
			LCD_KIT_ERR("tx cmd fail\n");
			return LCD_KIT_FAIL;
		}
	}

	ret = lcd_kit_dsi_cmds_rx(hisifd,
		(uint8_t *)disp_info->panel_version.read_value,
		&disp_info->panel_version.cmds);
	if (ret) {
		LCD_KIT_ERR("cmd fail\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->panel_version.exit_cmds.cmds != NULL) {
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->panel_version.exit_cmds);
		if (ret) {
			LCD_KIT_ERR("exit cmd fail\n");
			return LCD_KIT_FAIL;
		}
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
	/* gamma calibration */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,gamma-cal-support",
		&disp_info->gamma_cal.support, 0);
	if (disp_info->gamma_cal.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,gamma-cal-cmds",
			"lcd-kit,gamma-cal-cmds-state",
			&disp_info->gamma_cal.cmds);
	}
	/* brightness and color uniform */
	OF_PROPERTY_READ_U32_DEFAULT(np,
		"lcd-kit,brightness-color-uniform-support",
		&disp_info->oeminfo.brightness_color_uniform.support, 0);
	if (disp_info->oeminfo.brightness_color_uniform.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,brightness-color-cmds",
			"lcd-kit,brightness-color-cmds-state",
			&disp_info->oeminfo.brightness_color_uniform.brightness_color_cmds);
	/* oem information */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,oem-info-support",
		&disp_info->oeminfo.support, 0);
	if (disp_info->oeminfo.support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,oem-barcode-2d-support",
			&disp_info->oeminfo.barcode_2d.support, 0);
		if (disp_info->oeminfo.barcode_2d.support) {
			OF_PROPERTY_READ_U32_DEFAULT(np,
				"lcd-kit,oem-barcode-2d-block-num",
				&disp_info->oeminfo.barcode_2d.block_num, 3);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,barcode-2d-cmds",
				"lcd-kit,barcode-2d-cmds-state",
				&disp_info->oeminfo.barcode_2d.cmds);
		}
	}
	/* rgbw */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,rgbw-support",
		&disp_info->rgbw.support, 0);
	if (disp_info->rgbw.support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,rgbw-bl-max",
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
}

void lcd_kit_parse_util(struct device_node *np)
{
	char *name[VERSION_NUM_MAX] = {NULL};
	int i, ret;

	/* alpm */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,alpm-support", &disp_info->alpm.support, 0);
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
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,quickly-sleep-out-support",
		&disp_info->quickly_sleep_out.support, 0);
	if (disp_info->quickly_sleep_out.support)
		OF_PROPERTY_READ_U32_DEFAULT(np,
			"lcd-kit,quickly-sleep-out-interval",
			&disp_info->quickly_sleep_out.interval, 0);
	/* fps */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,fps-support",
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
	}
	/* project id */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,project-id-support",
		&disp_info->project_id.support, 0);
	if (disp_info->project_id.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,project-id-cmds",
			"lcd-kit,project-id-cmds-state",
			&disp_info->project_id.cmds);
		disp_info->project_id.default_project_id =
			(char *)of_get_property(np,
				"lcd-kit,default-project-id", NULL);
	}

	OF_PROPERTY_READ_U8_DEFAULT(np, "lcd-kit,cascade-ic-support",
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
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,panel-version-support",
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
		disp_info->panel_version.value_number =
			disp_info->panel_version.cmds.cmds->dlen -
			disp_info->panel_version.cmds.cmds->payload[1];
		lcd_kit_parse_arrays_data(np, "lcd-kit,panel-version-value",
			&disp_info->panel_version.value,
			disp_info->panel_version.value_number);
		disp_info->panel_version.version_number =
			disp_info->panel_version.value.cnt;

		LCD_KIT_INFO("Panel version value_num=%d version_num = %d\n",
			disp_info->panel_version.value_number,
			disp_info->panel_version.version_number);
		if (disp_info->panel_version.version_number > 0) {
			ret = of_property_read_string_array(np,
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
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,otp-gamma-support",
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
	/* pcd errflag */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,pcd-cmds-support",
		&disp_info->pcd_errflag.pcd_support, 0);
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
	}
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,errflag-cmds-support",
		&disp_info->pcd_errflag.errflag_support, 0);
	if (disp_info->pcd_errflag.errflag_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,errflag-read-cmds",
			"lcd-kit,errflag-read-cmds-state",
			&disp_info->pcd_errflag.read_errflag_cmds);
	}
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,dbv-stat-support",
		&disp_info->dbv_stat.support, 0);
	/* elvdd detect */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,elvdd-detect-support",
		&disp_info->elvdd_detect.support, 0);
	if (disp_info->elvdd_detect.support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,elvdd-detect-type",
			&disp_info->elvdd_detect.detect_type, 0);
		if (disp_info->elvdd_detect.detect_type ==
			ELVDD_MIPI_CHECK_MODE)
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,elvdd-detect-cmds",
				"lcd-kit,elvdd-detect-cmds-state",
				&disp_info->elvdd_detect.cmds);
		else
			OF_PROPERTY_READ_U32_DEFAULT(np,
				"lcd-kit,elvdd-detect-gpio",
				&disp_info->elvdd_detect.detect_gpio, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np,
			"lcd-kit,elvdd-detect-value",
			&disp_info->elvdd_detect.exp_value, 0);
		OF_PROPERTY_READ_U32_DEFAULT(np,
			"lcd-kit,elvdd-value-mask",
			&disp_info->elvdd_detect.exp_value_mask, 0);
	}
}

void lcd_kit_parse_dt(struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return;
	}
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,dsi1-support",
		&disp_info->dsi1_cmd_support, 0);
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
	uint32_t *expect_val, int cnt)
{
	int i;

	if (read_val == NULL || expect_val == NULL) {
		LCD_KIT_ERR("read_val or expect_val is NULL\n");
		return -1;
	}
	for (i = 0; i < cnt; i++) {
		if ((uint32_t)read_val[i] != expect_val[i])
			return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static void lcd_kit_pcd_dmd_report(uint8_t *pcd_read_val)
{
	int ret;
	char err_info[DMD_ERR_INFO_LEN] = {0};

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

int lcd_kit_check_pcd_errflag_check(struct hisi_fb_data_type *hisifd)
{
	uint8_t result = PCD_ERRFLAG_SUCCESS;
	int ret;
	uint8_t read_pcd[LCD_KIT_PCD_SIZE] = {0};
	uint8_t read_errflag[LCD_KIT_ERRFLAG_SIZE] = {0};
	uint32_t *expect_value = NULL;
	int i;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL\n");
		return -1;
	}
	if (disp_info->pcd_errflag.pcd_support) {
		(void)lcd_kit_dsi_cmds_tx(hisifd,
			&disp_info->pcd_errflag.switch_page_cmds);
		ret = lcd_kit_dsi_cmds_rx(hisifd, read_pcd,
			&disp_info->pcd_errflag.read_pcd_cmds);
		expect_value = disp_info->pcd_errflag.pcd_value.buf;
		if (ret == LCD_KIT_OK) {
			if (lcd_kit_judge_pcd_dmd(read_pcd, expect_value,
				LCD_KIT_PCD_SIZE) == LCD_KIT_OK) {
				lcd_kit_pcd_dmd_report(read_pcd);
				result |= PCD_FAIL;
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
	lcd_kit_dsi_cmds_rx(hisifd, (uint8_t *)read_value, &disp_info->gamma_cal.cmds);
	return LCD_KIT_OK;
}

void lcd_kit_read_power_status(struct hisi_fb_data_type *hisifd)
{
	uint32_t status;
	uint32_t try_times = 0;
	uint32_t status1;
	uint32_t try_times1 = 0;

	outp32(hisifd->mipi_dsi0_base + MIPIDSI_GEN_HDR_OFFSET, 0x0A06);
	status = inp32(hisifd->mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
	while (status & 0x10) {
		udelay(50);
		if (++try_times > 100) {
			try_times = 0;
			LCD_KIT_ERR("Read lcd power status timeout!\n");
			break;
		}

		status = inp32(hisifd->mipi_dsi0_base + MIPIDSI_CMD_PKT_STATUS_OFFSET);
	}
	status = inp32(hisifd->mipi_dsi0_base + MIPIDSI_GEN_PLD_DATA_OFFSET);
	LCD_KIT_INFO("LCD Power State = 0x%x. lcd_is_dual_mipi = 0x%x\n",
		status, lcd_is_dual_mipi());
	if (lcd_is_dual_mipi()) {
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
		strlen(disp_info->compatible))) {
		/* panel is online */
		status = LCD_OFFLINE;
	}
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
	if (is_mipi_cmd_panel(hisifd)) {
		/* wait fifo empty */
		(void)lcd_kit_dsi_fifo_is_empty(hisifd->mipi_dsi0_base);
		if (is_dual_mipi_panel(hisifd))
			lcd_kit_dsi_fifo_is_empty(hisifd->mipi_dsi1_base);
		LCD_KIT_INFO("link_state:%d\n", link_state);
		switch (link_state) {
		case LCD_KIT_DSI_LP_MODE:
			lcd_dsi_tx_lp_mode_cfg(hisifd->mipi_dsi0_base);
			lcd_dsi_rx_lp_mode_cfg(hisifd->mipi_dsi0_base);
			if (is_dual_mipi_panel(hisifd)) {
				lcd_dsi_tx_lp_mode_cfg(hisifd->mipi_dsi1_base);
				lcd_dsi_rx_lp_mode_cfg(hisifd->mipi_dsi1_base);
			}
			break;
		case LCD_KIT_DSI_HS_MODE:
			lcd_dsi_tx_hs_mode_cfg(hisifd->mipi_dsi0_base);
			lcd_dsi_rx_hs_mode_cfg(hisifd->mipi_dsi0_base);
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

int lcd_kit_get_value_from_dts(char *compatible, char *dts_name, u32 *value)
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
	OF_PROPERTY_READ_U32_RETURN(np, dts_name, value);
	return LCD_KIT_OK;
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
	down(&disp_info->thp_second_poweroff_sem);
	if (common_info->thp_proximity.panel_power_state == POWER_ON) {
		LCD_KIT_INFO("[Proximity_feature] power state is on exit!\n");
		up(&disp_info->thp_second_poweroff_sem);
		return LCD_KIT_FAIL;
	}
	if (common_info->thp_proximity.panel_power_state == POWER_TS_SUSPEND) {
		LCD_KIT_INFO("[Proximity_feature] power off suspend state exit!\n");
		up(&disp_info->thp_second_poweroff_sem);
		return LCD_KIT_OK;
	}
	if (common_info->thp_proximity.work_status == TP_PROXMITY_DISABLE) {
		LCD_KIT_INFO("[Proximity_feature] thp_proximity has been disabled exit!\n");
		up(&disp_info->thp_second_poweroff_sem);
		return LCD_KIT_FAIL;
	}
	common_info->thp_proximity.work_status = TP_PROXMITY_DISABLE;
	if (common_ops->panel_power_off)
		common_ops->panel_power_off(hisifd);
	up(&disp_info->thp_second_poweroff_sem);
	LCD_KIT_INFO("[Proximity_feature] lcd_kit_proximity_power_off exit!\n");
	return LCD_KIT_OK;
}

struct lcd_kit_ops g_lcd_ops = {
	.lcd_kit_support = lcd_kit_support,
	.get_project_id = lcd_kit_get_project_id,
	.get_status_by_type = lcd_kit_get_status_by_type,
#ifdef LCD_FACTORY_MODE
	.get_pt_station_status = lcd_kit_get_pt_station_status,
#endif
	.get_panel_power_status = lcd_kit_get_power_status,
	.power_monitor_on = lcd_kit_power_monitor_on,
	.power_monitor_off = lcd_kit_power_monitor_off,
	.set_vss_by_thermal = lcd_kit_set_vss_by_thermal,
	.write_otp_gamma = lcd_kit_write_otp_gamma,
	.proximity_power_off = lcd_kit_proximity_power_off,
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

int lcd_kit_utils_init(struct device_node *np, struct hisi_panel_info *pinfo)
{
	/* init sem */
	sema_init(&disp_info->lcd_kit_sem, 1);
	sema_init(&disp_info->thp_second_poweroff_sem, 1);
	/* init mipi lock */
	mutex_init(&disp_info->mipi_lock);
	/* parse display dts */
	lcd_kit_parse_dt(np);
	/* init hisi pinfo */
	lcd_kit_pinfo_init(np, pinfo);
	/* parse vesa parameters */
	lcd_kit_vesa_para_parse(np, pinfo);
	/* init compress config */
	lcd_kit_compress_config(pinfo->ifbc_type, pinfo);
	/* register lcd ops */
	lcd_kit_ops_register(&g_lcd_ops);
	/* effect init */
	lcd_kit_effect_get_data(lcd_kit_get_panel_id(disp_info->product_id,
		disp_info->compatible), pinfo);

	/* Read gamma data from shared memory */
	if (disp_info->gamma_cal.support) {
		hisifb_update_gm_from_reserved_mem(pinfo->gamma_lut_table_R,
			pinfo->gamma_lut_table_G, pinfo->gamma_lut_table_B,
			pinfo->igm_lut_table_R,	pinfo->igm_lut_table_G,
			pinfo->igm_lut_table_B);
	}

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

void lcd_hardware_reset(void)
{
	/* reset pull low */
	lcd_kit_reset_power_ctrl(0);
	msleep(300);
	/* reset pull high */
	lcd_kit_reset_power_ctrl(1);
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

bool lcd_is_dual_mipi(void)
{
	if (disp_info->dsi1_cmd_support)
		return true;

	return false;
}
