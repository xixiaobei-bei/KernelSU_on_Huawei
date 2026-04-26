/*
 * lcd_kit_debug.c
 *
 * lcdkit debug function for lcdkit head file
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

#include "lcd_kit_dbg.h"
#include <linux/string.h>
#include "lcd_kit_common.h"
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
#include "lcd_kit_dpd.h"
#include <base.h>

#define VALUE_MAX 3

static int lcd_kit_dbg_esd_support(char *par);
static int lcd_kit_dbg_fps_updt_support(char *par);
static int lcd_kit_dbg_quickly_sleep_out_support(char *par);
static int lcd_kit_dbg_dirty_region_support(char *par);
static int lcd_kit_dbg_blpwm_input_support(char *par);
static int lcd_kit_dbg_dsi_upt_support(char *par);
#ifdef LCD_FACTORY_MODE
static int lcd_kit_dbg_check_reg_support(char *par);
#endif
static int lcd_kit_dbg_effect_on_support(char *par);
static int lcd_kit_dbg_rgbw_support(char *par);
static int lcd_kit_dbg_cabc_support(char *par);
static int lcd_kit_dbg_gamma_support(char *par);
static int lcd_kit_dbg_gmp_support(char *par);
static int lcd_kit_dbg_hiace_support(char *par);
static int lcd_kit_dbg_xcc_support(char *par);
static int lcd_kit_dbg_arsr1psharpness_support(char *par);
static int lcd_kit_dbg_prefixsharptwo_d_support(char *par);
static int lcd_kit_dbg_video_idle_mode_support(char *par);
static int lcd_kit_dbg_cmd_type(char *par);
static int lcd_kit_dbg_pxl_clk(char *par);
static int lcd_kit_dbg_pxl_clk_div(char *par);
static int lcd_kit_dbg_vsync_ctrl_type(char *par);
static int lcd_kit_dbg_bl_max_nit(char *par);
static int lcd_kit_dbg_hback_porch(char *par);
static int lcd_kit_dbg_hfront_porch(char *par);
static int lcd_kit_dbg_hpulse_width(char *par);
static int lcd_kit_dbg_vback_porch(char *par);
static int lcd_kit_dbg_vfront_porch(char *par);
static int lcd_kit_dbg_vpulse_width(char *par);
static int lcd_kit_dbg_mipi_burst_mode(char *par);
static int lcd_kit_dbg_mipi_max_tx_esc_clk(char *par);
static int lcd_kit_dbg_mipi_dsi_bit_clk(char *par);
static int lcd_kit_dbg_mipi_dsi_bit_clk_a(char *par);
static int lcd_kit_dbg_mipi_dsi_bit_clk_b(char *par);
static int lcd_kit_dbg_mipi_dsi_bit_clk_c(char *par);
static int lcd_kit_dbg_mipi_dsi_bit_clk_d(char *par);
static int lcd_kit_dbg_mipi_dsi_bit_clk_e(char *par);
static int lcd_kit_dbg_mipi_noncontinue_enable(char *par);
static int lcd_kit_dbg_mipi_rg_vcm_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_post_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_pre_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_ths_prepare_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_tlpx_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_ths_trail_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_ths_exit_adjust(char *par);
static int lcd_kit_dbg_mipi_clk_ths_zero_adjust(char *par);
static int lcd_kit_dbg_mipi_lp11_flag(char *par);
static int lcd_kit_dbg_mipi_phy_update(char *par);
static int lcd_kit_dbg_power_on_stage(char *par);
static int lcd_kit_dbg_lp_on_stage(char *par);
static int lcd_kit_dbg_hs_on_stage(char *par);
static int lcd_kit_dbg_hs_off_stage(char *par);
static int lcd_kit_dbg_lp_off_stage(char *par);
static int lcd_kit_dbg_power_off_stage(char *par);
static int lcd_kit_dbg_on_cmd(char *par);
static int lcd_kit_dbg_off_cmd(char *par);
static int lcd_kit_dbg_effect_on_cmd(char *par);
static int lcd_kit_dbg_cabc_off_mode(char *par);
static int lcd_kit_dbg_cabc_ui_mode(char *par);
static int lcd_kit_dbg_cabc_still_mode(char *par);
static int lcd_kit_dbg_cabc_moving_mode(char *par);
static int lcd_kit_dbg_rgbw_bl_max(char *par);
static int lcd_kit_dbg_rgbw_set_mode1(char *par);
static int lcd_kit_dbg_rgbw_set_mode2(char *par);
static int lcd_kit_dbg_rgbw_set_mode3(char *par);
static int lcd_kit_dbg_rgbw_set_mode4(char *par);
static int lcd_kit_dbg_rgbw_backlight_cmd(char *par);
static int lcd_kit_dbg_rgbw_pixel_gainlimit_cmd(char *par);
static int lcd_kit_dbg_esd_reg_cmd(char *par);
static int lcd_kit_dbg_esd_value(char *par);
static int lcd_kit_dbg_dirty_region_cmd(char *par);
static int lcd_kit_dbg_barcode_2d_cmd(char *par);
static int lcd_kit_dbg_brightness_color_cmd(char *par);
static int lcd_kit_dbg_vci_voltage(char *par);
static int lcd_kit_dbg_iovcc_voltage(char *par);
static int lcd_kit_dbg_vdd_voltage(char *par);
static int lcd_kit_dbg_vsp_voltage(char *par);
static int lcd_kit_dbg_vsn_voltage(char *par);
static int lcd_kit_dbg_cmd(char *par);
static int lcd_kit_dbg_cmdstate(char *par);

struct lcd_kit_dbg_func item_func[] = {
	{ "PanelEsdSupport", lcd_kit_dbg_esd_support },
	{ "PanelFpsUpdtSupport", lcd_kit_dbg_fps_updt_support },
	{ "PanelQuicklySleepOutSupport", lcd_kit_dbg_quickly_sleep_out_support },
	{ "PanelDirtyRegionSupport", lcd_kit_dbg_dirty_region_support },
	{ "BlPwmInputDisable", lcd_kit_dbg_blpwm_input_support },
	{ "MipiDsiUptSupport", lcd_kit_dbg_dsi_upt_support },
#ifdef LCD_FACTORY_MODE
	{ "PanelCheckRegSupport", lcd_kit_dbg_check_reg_support },
#endif
	{ "PanelDisplayOnEffectSupport", lcd_kit_dbg_effect_on_support },
	{ "PanelRgbwSupport", lcd_kit_dbg_rgbw_support },
	{ "PanelCabcSupport", lcd_kit_dbg_cabc_support },
	{ "GammaSupport", lcd_kit_dbg_gamma_support },
	{ "GmpSupport", lcd_kit_dbg_gmp_support },
	{ "HiaceSupport", lcd_kit_dbg_hiace_support },
	{ "XccSupport", lcd_kit_dbg_xcc_support },
	{ "Arsr1pSharpnessSupport", lcd_kit_dbg_arsr1psharpness_support },
	{ "PrefixSharpTwoDSupport", lcd_kit_dbg_prefixsharptwo_d_support },
	{ "VideoIdleModeSupport", lcd_kit_dbg_video_idle_mode_support },
	{ "PanelCmdType", lcd_kit_dbg_cmd_type },
	{ "PanelPxlClk", lcd_kit_dbg_pxl_clk },
	{ "PanelPxlClkDiv", lcd_kit_dbg_pxl_clk_div },
	{ "PanelVsynCtrType", lcd_kit_dbg_vsync_ctrl_type },
	{ "PanelBlMaxnit", lcd_kit_dbg_bl_max_nit },
	{ "HBackPorch", lcd_kit_dbg_hback_porch },
	{ "HFrontPorch", lcd_kit_dbg_hfront_porch },
	{ "HPulseWidth", lcd_kit_dbg_hpulse_width },
	{ "VBackPorch", lcd_kit_dbg_vback_porch },
	{ "VFrontPorch", lcd_kit_dbg_vfront_porch },
	{ "VPulseWidth", lcd_kit_dbg_vpulse_width },
	{ "MipiBurstMode", lcd_kit_dbg_mipi_burst_mode },
	{ "MipiMaxTxEscClk", lcd_kit_dbg_mipi_max_tx_esc_clk },
	{ "MipiDsiBitClk", lcd_kit_dbg_mipi_dsi_bit_clk },
	{ "MipiDsiBitClkValA", lcd_kit_dbg_mipi_dsi_bit_clk_a },
	{ "MipiDsiBitClkValB", lcd_kit_dbg_mipi_dsi_bit_clk_b },
	{ "MipiDsiBitClkValC", lcd_kit_dbg_mipi_dsi_bit_clk_c },
	{ "MipiDsiBitClkValD", lcd_kit_dbg_mipi_dsi_bit_clk_d },
	{ "MipiDsiBitClkValE", lcd_kit_dbg_mipi_dsi_bit_clk_e },
	{ "MipiNonContinueEnable", lcd_kit_dbg_mipi_noncontinue_enable },
	{ "MipiRgVcmAdjust", lcd_kit_dbg_mipi_rg_vcm_adjust },
	{ "MipiClkPostAdjust", lcd_kit_dbg_mipi_clk_post_adjust },
	{ "MipiClkPreAdjust", lcd_kit_dbg_mipi_clk_pre_adjust },
	{ "MipiClkThsPrepareAdjust", lcd_kit_dbg_mipi_clk_ths_prepare_adjust },
	{ "MipiClkTlpxAdjust", lcd_kit_dbg_mipi_clk_tlpx_adjust },
	{ "MipiClkThsTrailAdjust", lcd_kit_dbg_mipi_clk_ths_trail_adjust },
	{ "MipiClkThsExitAdjust", lcd_kit_dbg_mipi_clk_ths_exit_adjust },
	{ "MipiClkThsZeroAdjust", lcd_kit_dbg_mipi_clk_ths_zero_adjust },
	{ "MipiLp11Flag", lcd_kit_dbg_mipi_lp11_flag },
	{ "MipiPhyUpdate", lcd_kit_dbg_mipi_phy_update },
	{ "PowerOnStage", lcd_kit_dbg_power_on_stage },
	{ "LPOnStage", lcd_kit_dbg_lp_on_stage },
	{ "HSOnStage", lcd_kit_dbg_hs_on_stage },
	{ "HSOffStage", lcd_kit_dbg_hs_off_stage },
	{ "LPOffStage", lcd_kit_dbg_lp_off_stage },
	{ "PowerOffStage", lcd_kit_dbg_power_off_stage },
	{ "PanelOnCommand", lcd_kit_dbg_on_cmd },
	{ "PanelOffCommand", lcd_kit_dbg_off_cmd },
	{ "PanelDisplayOnEffectCommand", lcd_kit_dbg_effect_on_cmd },
	{ "PanelCabcOffMode", lcd_kit_dbg_cabc_off_mode },
	{ "PanelCabcUiMode", lcd_kit_dbg_cabc_ui_mode },
	{ "PanelCabcStillMode", lcd_kit_dbg_cabc_still_mode },
	{ "PanelCabcMovingMode", lcd_kit_dbg_cabc_moving_mode },
	{ "PanelRgbwBlMax", lcd_kit_dbg_rgbw_bl_max },
	{ "PanelRgbwSet1Mode", lcd_kit_dbg_rgbw_set_mode1 },
	{ "PanelRgbwSet2Mode", lcd_kit_dbg_rgbw_set_mode2 },
	{ "PanelRgbwSet3Mode", lcd_kit_dbg_rgbw_set_mode3 },
	{ "PanelRgbwSet4Mode", lcd_kit_dbg_rgbw_set_mode4 },
	{ "PanelRgbwBacklightCommand", lcd_kit_dbg_rgbw_backlight_cmd },
	{ "PanelRgbwPixelgainlimitCommand", lcd_kit_dbg_rgbw_pixel_gainlimit_cmd },
	{ "PanelEsdRegCommand", lcd_kit_dbg_esd_reg_cmd },
	{ "PanelEsdValue", lcd_kit_dbg_esd_value },
	{ "PanelDirtyRegionCommand", lcd_kit_dbg_dirty_region_cmd },
	{ "Barcode2DCommand", lcd_kit_dbg_barcode_2d_cmd },
	{ "PanelBrightnessColorCommand", lcd_kit_dbg_brightness_color_cmd },
	{ "LcdVci", lcd_kit_dbg_vci_voltage },
	{ "LcdIovcc", lcd_kit_dbg_iovcc_voltage },
	{ "LcdVdd", lcd_kit_dbg_vdd_voltage },
	{ "LcdVsp", lcd_kit_dbg_vsp_voltage },
	{ "LcdVsn", lcd_kit_dbg_vsn_voltage },
	/* send mipi cmds for debugging, both support tx and rx */
	{ "PanelDbgCommand", lcd_kit_dbg_cmd },
	{ "PanelDbgCommandState", lcd_kit_dbg_cmdstate },
};

struct lcd_kit_dbg_cmds lcd_kit_cmd_list[] = {
	{ LCD_KIT_DBG_LEVEL_SET, "set_debug_level" },
	{ LCD_KIT_DBG_PARAM_CONFIG, "set_param_config" },
};

struct lcd_kit_debug lcd_kit_dbg;
/* show usage or print last read result */
static char lcd_kit_debug_buf[LCD_KIT_DBG_BUFF_MAX];

static struct lcd_kit_dbg_ops *g_dbg_ops;

int lcd_kit_debug_register(struct lcd_kit_dbg_ops *ops)
{
	if (g_dbg_ops) {
		LCD_KIT_ERR("g_dbg_ops has already been registered!\n");
		return LCD_KIT_FAIL;
	}
	g_dbg_ops = ops;
	LCD_KIT_INFO("g_dbg_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_debug_unregister(struct lcd_kit_dbg_ops *ops)
{
	if (g_dbg_ops == ops) {
		g_dbg_ops = NULL;
		LCD_KIT_INFO("g_dbg_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_dbg_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

bool lcd_kit_is_valid_char(char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	if (ch >= 'a' && ch <= 'f')
		return true;
	if (ch >= 'A' && ch <= 'F')
		return true;
	return false;
}

bool is_valid_char(char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	if (ch >= 'a' && ch <= 'z')
		return true;
	if (ch >= 'A' && ch <= 'Z')
		return true;
	return false;
}

struct lcd_kit_dbg_ops *lcd_kit_get_debug_ops(void)
{
	return g_dbg_ops;
}

static char lcd_kit_hex_char_to_value(char ch)
{
	switch (ch) {
	case 'a' ... 'f':
		ch = 10 + (ch - 'a');
		break;
	case 'A' ... 'F':
		ch = 10 + (ch - 'A');
		break;
	case '0' ... '9':
		ch = ch - '0';
		break;
	}

	return ch;
}

void lcd_kit_dump_buf(const char *buf, int cnt)
{
	int i;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return;
	}
	for (i = 0; i < cnt; i++)
		LCD_KIT_DEBUG("buf[%d] = 0x%02x\n", i, buf[i]);
}

void lcd_kit_dump_buf_32(const u32 *buf, int cnt)
{
	int i = 0;

	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return;
	}
	for (i = 0; i < cnt; i++)
		LCD_KIT_DEBUG("buf[%d] = 0x%02x\n", i, buf[i]);

}

void lcd_kit_dump_cmds_desc(struct lcd_kit_dsi_cmd_desc *desc)
{
	if (!desc) {
		LCD_KIT_INFO("NULL point!\n");
		return;
	}
	LCD_KIT_DEBUG("dtype      = 0x%02x\n", desc->dtype);
	LCD_KIT_DEBUG("last       = 0x%02x\n", desc->last);
	LCD_KIT_DEBUG("vc         = 0x%02x\n", desc->vc);
	LCD_KIT_DEBUG("ack        = 0x%02x\n", desc->ack);
	LCD_KIT_DEBUG("wait       = 0x%02x\n", desc->wait);
	LCD_KIT_DEBUG("waittype   = 0x%02x\n", desc->waittype);
	LCD_KIT_DEBUG("dlen       = 0x%02x\n", desc->dlen);

	lcd_kit_dump_buf(desc->payload, (int)(desc->dlen));
}

void lcd_kit_dump_cmds(struct lcd_kit_dsi_panel_cmds *cmds)
{
	int i;

	if (!cmds) {
		LCD_KIT_INFO("NULL point!\n");
		return;
	}
	LCD_KIT_DEBUG("blen       = 0x%02x\n", cmds->blen);
	LCD_KIT_DEBUG("cmd_cnt    = 0x%02x\n", cmds->cmd_cnt);
	LCD_KIT_DEBUG("link_state = 0x%02x\n", cmds->link_state);
	LCD_KIT_DEBUG("flags      = 0x%02x\n", cmds->flags);
	for (i = 0; i < cmds->cmd_cnt; i++)
		lcd_kit_dump_cmds_desc(&cmds->cmds[i]);
}

/* convert string to lower case */
/* return: 0 - success, negative - fail */
static int lcd_kit_str_to_lower(char *str)
{
	char *tmp = str;

	/* check param */
	if (!tmp)
		return -1;
	while (*tmp != '\0') {
		*tmp = tolower(*tmp);
		tmp++;
	}
	return 0;
}

/* check if string start with sub string */
/* return: 0 - success, negative - fail */
static int lcd_kit_str_start_with(const char *str, const char *sub)
{
	/* check param */
	if (!str || !sub)
		return -EINVAL;
	return ((strncmp(str, sub, strlen(sub)) == 0) ? 0 : -1);
}

int lcd_kit_str_to_del_invalid_ch(char *str)
{
	char *tmp = str;

	/* check param */
	if (!tmp)
		return -1;
	while (*str != '\0') {
		if (lcd_kit_is_valid_char(*str) || *str == ',' || *str == 'x' ||
			*str == 'X') {
			*tmp = *str;
			tmp++;
		}
		str++;
	}
	*tmp = '\0';
	return 0;
}

int lcd_kit_str_to_del_ch(char *str, char ch)
{
	char *tmp = str;

	/* check param */
	if (!tmp)
		return -1;
	while (*str != '\0') {
		if (*str != ch) {
			*tmp = *str;
			tmp++;
		}
		str++;
	}
	*tmp = '\0';
	return 0;
}

/* parse config xml */
int lcd_kit_parse_u8_digit(char *in, char *out, int max)
{
	unsigned char ch = '\0';
	unsigned char last_char = 'Z';
	unsigned char last_ch = 'Z';
	int j = 0;
	int i = 0;
	int len;

	if (!in || !out) {
		LCD_KIT_ERR("in or out is null\n");
		return LCD_KIT_FAIL;
	}
	len = strlen(in);
	LCD_KIT_INFO("LEN = %d\n", len);
	while (len--) {
		ch = in[i++];
		if (last_ch == '0' && ((ch == 'x') || (ch == 'X'))) {
			j--;
			last_char = 'Z';
			continue;
		}
		last_ch = ch;
		if (!lcd_kit_is_valid_char(ch)) {
			last_char = 'Z';
			continue;
		}
		if (last_char != 'Z') {
			/*
			 * two char value is possible like F0,
			 * so make it a single char
			 */
			--j;
			if (j >= max) {
				LCD_KIT_ERR("number is too much\n");
				return LCD_KIT_FAIL;
			}
			out[j] = (out[j] * LCD_KIT_HEX_BASE) +
				lcd_kit_hex_char_to_value(ch);
			last_char = 'Z';
		} else {
			if (j >= max) {
				LCD_KIT_ERR("number is too much\n");
				return LCD_KIT_FAIL;
			}
			out[j] = lcd_kit_hex_char_to_value(ch);
			last_char = out[j];
		}
		j++;
	}
	return j;
}

int lcd_kit_parse_u32_digit(char *in, unsigned int *out, int len)
{
	char *delim = ",";
	int i = 0;
	char *str1 = NULL;
	char *str2 = NULL;

	if (!in || !out) {
		LCD_KIT_ERR("in or out is null\n");
		return LCD_KIT_FAIL;
	}

	lcd_kit_str_to_del_invalid_ch(in);
	str1 = in;
	do {
		str2 = strstr(str1, delim);
		if (i >= len) {
			LCD_KIT_ERR("number is too much\n");
			return LCD_KIT_FAIL;
		}
		if (str2 == NULL) {
			out[i++] = simple_strtoul(str1, NULL, 0);
			break;
		}
		*str2 = 0;
		out[i++] = simple_strtoul(str1, NULL, 0);
		str2++;
		str1 = str2;
	} while (str2 != NULL);
	return i;
}

int lcd_kit_dbg_parse_array(char *in, unsigned int *array,
	struct lcd_kit_arrays_data *out, int len)
{
	char *delim = "\n";
	int count = 0;
	char *str1 = NULL;
	char *str2 = NULL;
	unsigned int *temp = NULL;
	struct lcd_kit_array_data *tmp = NULL;

	if (!in || !array || !out) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	temp = array;
	str1 = in;
	tmp = out->arry_data;
	if (!temp || !str1 || !tmp) {
		LCD_KIT_ERR("temp or str1 or tmp is null\n");
		return LCD_KIT_FAIL;
	}
	do {
		str2 = strstr(str1, delim);
		if (str2 == NULL) {
			lcd_kit_parse_u32_digit(str1, temp, len);
			tmp->buf = temp;
			tmp++;
			temp += len;
			count++;
			break;
		}
		*str2 = 0;
		lcd_kit_parse_u32_digit(str1, temp, len);
		tmp->buf = temp;
		tmp++;
		temp += len;
		count++;
		str2++;
		str1 = str2;
	} while (str2 != NULL);
	out->cnt = count;
	LCD_KIT_INFO("out->cnt = %d\n", out->cnt);
	return count;
}

int lcd_kit_dbg_parse_cmd(struct lcd_kit_dsi_panel_cmds *pcmds, char *buf,
	int length)
{
	int blen;
	int len;
	char *bp = NULL;
	struct lcd_kit_dsi_ctrl_hdr *dchdr = NULL;
	struct lcd_kit_dsi_cmd_desc *newcmds = NULL;
	int i;
	int cnt = 0;

	if (!pcmds || !buf) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	/* scan dcs commands */
	bp = buf;
	blen = length;
	len = blen;
	while (len > sizeof(*dchdr)) {
		dchdr = (struct lcd_kit_dsi_ctrl_hdr *)bp;
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		if (dchdr->dlen > len) {
			LCD_KIT_ERR("dtsi cmd=%x error, len=%d, cnt=%d\n",
				dchdr->dtype, dchdr->dlen, cnt);
			return LCD_KIT_FAIL;
		}
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}
	if (len != 0) {
		LCD_KIT_ERR("dcs_cmd=%x len=%d error!\n", buf[0], blen);
		return LCD_KIT_FAIL;
	}
	newcmds = kzalloc(cnt * sizeof(*newcmds), GFP_KERNEL);
	if (newcmds == NULL) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (pcmds->cmds != NULL)
		kfree(pcmds->cmds);
	pcmds->cmds = newcmds;
	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;
	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct lcd_kit_dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dtype = dchdr->dtype;
		pcmds->cmds[i].last = dchdr->last;
		pcmds->cmds[i].vc = dchdr->vc;
		pcmds->cmds[i].ack = dchdr->ack;
		pcmds->cmds[i].wait = dchdr->wait;
		pcmds->cmds[i].waittype = dchdr->waittype;
		pcmds->cmds[i].dlen = dchdr->dlen;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}
	lcd_kit_dump_cmds(pcmds);
	return 0;
}

void lcd_kit_dbg_free(void)
{
	kfree(lcd_kit_dbg.dbg_esd_cmds);
	kfree(lcd_kit_dbg.dbg_on_cmds);
	kfree(lcd_kit_dbg.dbg_off_cmds);
	kfree(lcd_kit_dbg.dbg_effect_on_cmds);
	kfree(lcd_kit_dbg.dbg_cabc_off_cmds);
	kfree(lcd_kit_dbg.dbg_cabc_ui_cmds);
	kfree(lcd_kit_dbg.dbg_cabc_still_cmds);
	kfree(lcd_kit_dbg.dbg_cabc_moving_cmds);
	kfree(lcd_kit_dbg.dbg_rgbw_mode1_cmds);
	kfree(lcd_kit_dbg.dbg_rgbw_mode2_cmds);
	kfree(lcd_kit_dbg.dbg_rgbw_mode3_cmds);
	kfree(lcd_kit_dbg.dbg_rgbw_mode4_cmds);
	kfree(lcd_kit_dbg.dbg_rgbw_backlight_cmds);
	kfree(lcd_kit_dbg.dbg_rgbw_pixel_gainlimit_cmds);
	kfree(lcd_kit_dbg.dbg_dirty_region_cmds);
	kfree(lcd_kit_dbg.dbg_barcode_2d_cmds);
	kfree(lcd_kit_dbg.dbg_brightness_color_cmds);
	kfree(lcd_kit_dbg.dbg_power_on_array);
}

static int lcd_kit_dbg_esd_support(char *par)
{
	char ch;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null!\n");
		return LCD_KIT_FAIL;
	}

	ch = *par;
	common_info->esd.support = lcd_kit_hex_char_to_value(ch);
	if (common_info->esd.support && dbg_ops->esd_check_func)
		dbg_ops->esd_check_func();
	LCD_KIT_INFO("esd.support = %d\n", common_info->esd.support);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_fps_updt_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->fps_updt_support)
		dbg_ops->fps_updt_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_quickly_sleep_out_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->quickly_sleep_out_support)
		dbg_ops->quickly_sleep_out_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_dirty_region_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	common_info->dirty_region.support = value;
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_blpwm_input_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->blpwm_input_support)
		dbg_ops->blpwm_input_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_dsi_upt_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->dsi_upt_support)
		dbg_ops->dsi_upt_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

#ifdef LCD_FACTORY_MODE
static int lcd_kit_dbg_check_reg_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	FACT_INFO->check_reg.support = value;
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}
#endif

static int lcd_kit_dbg_effect_on_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	common_info->effect_on.support = value;
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->rgbw_support)
		dbg_ops->rgbw_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_cabc_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	common_info->cabc.support = value;
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_gamma_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->gamma_support)
		dbg_ops->gamma_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_gmp_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->gmp_support)
		dbg_ops->gmp_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_hiace_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->hiace_support)
		dbg_ops->hiace_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_xcc_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->xcc_support)
		dbg_ops->xcc_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_arsr1psharpness_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->arsr1psharpness_support)
		dbg_ops->arsr1psharpness_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_prefixsharptwo_d_support(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->prefixsharptwo_d_support)
		dbg_ops->prefixsharptwo_d_support(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_video_idle_mode_support(char *par)
{
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	value = lcd_kit_hex_char_to_value(*par);
	if (dbg_ops->video_idle_mode_support)
		dbg_ops->video_idle_mode_support(value);
	LCD_KIT_INFO("video idle mode support = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_cmd_type(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->cmd_type)
		dbg_ops->cmd_type(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_pxl_clk(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->pxl_clk)
		dbg_ops->pxl_clk(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_pxl_clk_div(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->pxl_clk_div)
		dbg_ops->pxl_clk_div(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_vsync_ctrl_type(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->vsync_ctrl_type)
		dbg_ops->vsync_ctrl_type(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_bl_max_nit(char *par)
{
	int value = 0;

	lcd_kit_parse_u32_digit(par, &value, 1);
	common_info->bl_max_nit = value;
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_hback_porch(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->hback_porch)
		dbg_ops->hback_porch(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_hfront_porch(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->hfront_porch)
		dbg_ops->hfront_porch(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_hpulse_width(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->hpulse_width)
		dbg_ops->hpulse_width(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_vback_porch(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->vback_porch)
		dbg_ops->vback_porch(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_vfront_porch(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->vfront_porch)
		dbg_ops->vfront_porch(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_vpulse_width(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->vpulse_width)
		dbg_ops->vpulse_width(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_burst_mode(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_burst_mode)
		dbg_ops->mipi_burst_mode(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_max_tx_esc_clk(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_max_tx_esc_clk)
		dbg_ops->mipi_max_tx_esc_clk(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_dsi_bit_clk(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_dsi_bit_clk)
		dbg_ops->mipi_dsi_bit_clk(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_dsi_bit_clk_a(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_dsi_bit_clk_a)
		dbg_ops->mipi_dsi_bit_clk_a(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_dsi_bit_clk_b(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_dsi_bit_clk_b)
		dbg_ops->mipi_dsi_bit_clk_b(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_dsi_bit_clk_c(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_dsi_bit_clk_c)
		dbg_ops->mipi_dsi_bit_clk_c(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_dsi_bit_clk_d(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_dsi_bit_clk_d)
		dbg_ops->mipi_dsi_bit_clk_d(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_dsi_bit_clk_e(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_dsi_bit_clk_e)
		dbg_ops->mipi_dsi_bit_clk_e(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_noncontinue_enable(char *par)
{
	char ch;
	int value;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	ch = *par;
	value = lcd_kit_hex_char_to_value(ch);
	if (dbg_ops->mipi_noncontinue_enable)
		dbg_ops->mipi_noncontinue_enable(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_rg_vcm_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_rg_vcm_adjust)
		dbg_ops->mipi_rg_vcm_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_post_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_post_adjust)
		dbg_ops->mipi_clk_post_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_pre_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_pre_adjust)
		dbg_ops->mipi_clk_pre_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_ths_prepare_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_ths_prepare_adjust)
		dbg_ops->mipi_clk_ths_prepare_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_tlpx_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_tlpx_adjust)
		dbg_ops->mipi_clk_tlpx_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_ths_trail_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_ths_trail_adjust)
		dbg_ops->mipi_clk_ths_trail_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_ths_exit_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_ths_exit_adjust)
		dbg_ops->mipi_clk_ths_exit_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_clk_ths_zero_adjust(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_clk_ths_zero_adjust)
		dbg_ops->mipi_clk_ths_zero_adjust(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_lp11_flag(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_lp11_flag)
		dbg_ops->mipi_lp11_flag(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_mipi_phy_update(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->mipi_phy_update)
		dbg_ops->mipi_phy_update(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_power_on_stage(char *par)
{
	struct lcd_kit_array_data *temp_arry = NULL;
	int i;
	int j = 0;
	char ch = '"';

	lcd_kit_dbg.dbg_power_on_array = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_power_on_array) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(par, ch);
	if (power_seq == NULL) {
		LCD_KIT_ERR("power_seq is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(par, lcd_kit_dbg.dbg_power_on_array,
		&power_seq->power_on_seq, SEQ_NUM);
	temp_arry = power_seq->power_on_seq.arry_data;
	if (!temp_arry) {
		LCD_KIT_ERR("temp_arry is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < power_seq->power_on_seq.cnt; i++) {
		if (!temp_arry || !temp_arry->buf) {
			LCD_KIT_ERR("temp_arry or temp_arry->buf is null!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("power_on_seq.arry_data->buf[0] = %d\n",
			temp_arry->buf[EVENT_NUM]);
		LCD_KIT_INFO("power_on_seq.arry_data->buf[1] = %d\n",
			temp_arry->buf[EVENT_DATA]);
		LCD_KIT_INFO("power_on_seq.arry_data->buf[2] = %d\n",
			temp_arry->buf[EVENT_DELAY]);
		temp_arry++;
		j += SEQ_NUM;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_lp_on_stage(char *par)
{
	struct lcd_kit_array_data *temp_arry = NULL;
	int i;
	int j = 0;
	char ch = '"';

	lcd_kit_dbg.dbg_lp_on_array = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_lp_on_array) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(par, ch);
	if (power_seq == NULL) {
		LCD_KIT_ERR("power_seq is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(par, lcd_kit_dbg.dbg_lp_on_array,
		&power_seq->panel_on_lp_seq, SEQ_NUM);
	temp_arry = power_seq->panel_on_lp_seq.arry_data;
	if (!temp_arry) {
		LCD_KIT_ERR("temp_arry is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < power_seq->panel_on_lp_seq.cnt; i++) {
		if (!temp_arry || !temp_arry->buf) {
			LCD_KIT_ERR("temp_arry or temp_arry->buf is null!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("panel_on_lp_seq.arry_data->buf[0] = %d\n",
			temp_arry->buf[EVENT_NUM]);
		LCD_KIT_INFO("panel_on_lp_seq.arry_data->buf[1] = %d\n",
			temp_arry->buf[EVENT_DATA]);
		LCD_KIT_INFO("panel_on_lp_seq.arry_data->buf[2] = %d\n",
			temp_arry->buf[EVENT_DELAY]);
		temp_arry++;
		j += SEQ_NUM;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_hs_on_stage(char *par)
{
	struct lcd_kit_array_data *temp_arry = NULL;
	int i;
	int j = 0;
	char ch = '"';

	lcd_kit_dbg.dbg_hs_on_array = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_hs_on_array) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(par, ch);
	if (power_seq == NULL) {
		LCD_KIT_ERR("power_seq is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(par, lcd_kit_dbg.dbg_hs_on_array,
		&power_seq->panel_on_hs_seq, SEQ_NUM);
	temp_arry = power_seq->panel_on_hs_seq.arry_data;
	if (!temp_arry) {
		LCD_KIT_ERR("temp_arry is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < power_seq->panel_on_hs_seq.cnt; i++) {
		if (!temp_arry || !temp_arry->buf) {
			LCD_KIT_ERR("temp_arry or temp_arry->buf is null!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("panel_on_hs_seq.arry_data->buf[0] = %d\n",
			temp_arry->buf[EVENT_NUM]);
		LCD_KIT_INFO("panel_on_hs_seq.arry_data->buf[1] = %d\n",
			temp_arry->buf[EVENT_DATA]);
		LCD_KIT_INFO("panel_on_hs_seq.arry_data->buf[2] = %d\n",
			temp_arry->buf[EVENT_DELAY]);
		temp_arry++;
		j += SEQ_NUM;
	}
	return LCD_KIT_OK;
}


static int lcd_kit_dbg_hs_off_stage(char *par)
{
	struct lcd_kit_array_data *temp_arry = NULL;
	int i;
	int j = 0;
	char ch = '"';

	lcd_kit_dbg.dbg_hs_off_array = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_hs_off_array) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(par, ch);
	if (power_seq == NULL) {
		LCD_KIT_ERR("power_seq is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(par, lcd_kit_dbg.dbg_hs_off_array,
		&power_seq->panel_off_hs_seq, SEQ_NUM);
	temp_arry = power_seq->panel_off_hs_seq.arry_data;
	if (!temp_arry) {
		LCD_KIT_ERR("temp_arry is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < power_seq->panel_off_hs_seq.cnt; i++) {
		if (!temp_arry || !temp_arry->buf) {
			LCD_KIT_ERR("temp_arry or temp_arry->buf is null!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("panel_off_hs_seq.arry_data->buf[0] = %d\n",
			temp_arry->buf[EVENT_NUM]);
		LCD_KIT_INFO("panel_off_hs_seq.arry_data->buf[1] = %d\n",
			temp_arry->buf[EVENT_DATA]);
		LCD_KIT_INFO("panel_off_hs_seq.arry_data->buf[2] = %d\n",
			temp_arry->buf[EVENT_DELAY]);
		temp_arry++;
		j += SEQ_NUM;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_lp_off_stage(char *par)
{
	struct lcd_kit_array_data *temp_arry = NULL;
	int i;
	int j = 0;
	char ch = '"';

	lcd_kit_dbg.dbg_lp_off_array = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_lp_off_array) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(par, ch);
	if (power_seq == NULL) {
		LCD_KIT_ERR("power_seq is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(par, lcd_kit_dbg.dbg_lp_off_array,
		&power_seq->panel_off_lp_seq, SEQ_NUM);
	temp_arry = power_seq->panel_off_lp_seq.arry_data;
	if (!temp_arry) {
		LCD_KIT_ERR("temp_arry is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < power_seq->panel_off_lp_seq.cnt; i++) {
		if (!temp_arry || !temp_arry->buf) {
			LCD_KIT_ERR("temp_arry or temp_arry->buf is null!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("panel_off_lp_seq.arry_data->buf[0] = %d\n",
			temp_arry->buf[EVENT_NUM]);
		LCD_KIT_INFO("panel_off_lp_seq.arry_data->buf[1] = %d\n",
			temp_arry->buf[EVENT_DATA]);
		LCD_KIT_INFO("panel_off_lp_seq.arry_data->buf[2] = %d\n",
			temp_arry->buf[EVENT_DELAY]);
		temp_arry++;
		j += SEQ_NUM;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_power_off_stage(char *par)
{
	struct lcd_kit_array_data *temp_arry = NULL;
	int i;
	int j = 0;
	char ch = '"';

	lcd_kit_dbg.dbg_power_off_array = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_power_off_array) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(par, ch);
	if (power_seq == NULL) {
		LCD_KIT_ERR("power_seq is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(par, lcd_kit_dbg.dbg_power_off_array,
		&power_seq->power_off_seq, SEQ_NUM);
	temp_arry = power_seq->power_off_seq.arry_data;
	if (!temp_arry) {
		LCD_KIT_ERR("temp_arry is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < power_seq->power_off_seq.cnt; i++) {
		if (!temp_arry || !temp_arry->buf) {
			LCD_KIT_ERR("temp_arry or temp_arry->buf is null!\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_INFO("power_off_seq.arry_data->buf[0] = %d\n",
			temp_arry->buf[EVENT_NUM]);
		LCD_KIT_INFO("power_off_seq.arry_data->buf[1] = %d\n",
			temp_arry->buf[EVENT_DATA]);
		LCD_KIT_INFO("power_off_seq.arry_data->buf[2] = %d\n",
			temp_arry->buf[EVENT_DELAY]);
		temp_arry++;
		j += SEQ_NUM;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_on_cmd(char *par)
{
	int len;

	lcd_kit_dbg.dbg_on_cmds = kzalloc(LCD_KIT_CONFIG_TABLE_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_on_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_on_cmds,
		LCD_KIT_CONFIG_TABLE_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->panel_on_cmds,
			lcd_kit_dbg.dbg_on_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_off_cmd(char *par)
{
	int len;

	lcd_kit_dbg.dbg_off_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_off_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_off_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->panel_off_cmds,
			lcd_kit_dbg.dbg_off_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_effect_on_cmd(char *par)
{
	int len;

	lcd_kit_dbg.dbg_effect_on_cmds = kzalloc(LCD_KIT_CONFIG_TABLE_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_effect_on_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_effect_on_cmds,
		LCD_KIT_CONFIG_TABLE_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->effect_on.cmds,
			lcd_kit_dbg.dbg_effect_on_cmds, len);
	return LCD_KIT_OK;
}

static struct lcd_kit_dsi_panel_cmds dbgcmds;
static int lcd_kit_dbg_cmd(char *par)
{
#define LCD_DDIC_INFO_LEN 200
#define PRI_LINE_LEN 8
	struct lcd_kit_dbg_ops *dbg_ops = NULL;
	uint8_t readbuf[LCD_DDIC_INFO_LEN] = {0};
	int len, i;

	dbgcmds.buf = NULL;
	dbgcmds.blen = 0;
	dbgcmds.cmds = NULL;
	dbgcmds.cmd_cnt = 0;
	dbgcmds.flags = 0;
	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null!\n");
		return LCD_KIT_FAIL;
	}
	if (dbg_ops->panel_power_on) {
		if (!dbg_ops->panel_power_on()) {
			LCD_KIT_ERR("panel power off!\n");
			return LCD_KIT_FAIL;
		}
	} else {
		LCD_KIT_ERR("panel_power_on is null!\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_cmds = kzalloc(LCD_KIT_CONFIG_TABLE_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_cmds,
		LCD_KIT_CONFIG_TABLE_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&dbgcmds, lcd_kit_dbg.dbg_cmds, len);
	if (dbg_ops->dbg_mipi_rx) {
		dbg_ops->dbg_mipi_rx(readbuf, LCD_DDIC_INFO_LEN, &dbgcmds);
		readbuf[LCD_DDIC_INFO_LEN - 1] = '\0';
		LCD_KIT_INFO("dbg-cmd read string:%s\n", readbuf);
		LCD_KIT_INFO("corresponding hex data:\n");
		for (i = 0; i < LCD_DDIC_INFO_LEN; i++) {
			LCD_KIT_INFO("0x%x", readbuf[i]);
			if ((i + 1) % PRI_LINE_LEN == 0)
				LCD_KIT_INFO("\n");
		}
		LCD_KIT_INFO("dbg_mipi_rx done\n");
		kfree(lcd_kit_dbg.dbg_cmds);
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("dbg_mipi_rx is NULL!\n");
	kfree(lcd_kit_dbg.dbg_cmds);
	return LCD_KIT_FAIL;
}

static int lcd_kit_dbg_cmdstate(char *par)
{
	if (!par) {
		LCD_KIT_ERR("par is null\n");
		return LCD_KIT_FAIL;
	}
	dbgcmds.link_state = lcd_kit_hex_char_to_value(*par);
	LCD_KIT_INFO("dbgcmds link_state = %d\n", dbgcmds.link_state);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_cabc_off_mode(char *par)
{
	int len;

	lcd_kit_dbg.dbg_cabc_off_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_cabc_off_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_cabc_off_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->cabc.cabc_off_cmds,
			lcd_kit_dbg.dbg_cabc_off_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_cabc_ui_mode(char *par)
{
	int len;

	lcd_kit_dbg.dbg_cabc_ui_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_cabc_ui_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_cabc_ui_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->cabc.cabc_ui_cmds,
			lcd_kit_dbg.dbg_cabc_ui_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_cabc_still_mode(char *par)
{
	int len;

	lcd_kit_dbg.dbg_cabc_still_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_cabc_still_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_cabc_still_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->cabc.cabc_still_cmds,
			lcd_kit_dbg.dbg_cabc_still_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_cabc_moving_mode(char *par)
{
	int len;

	lcd_kit_dbg.dbg_cabc_moving_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_cabc_moving_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_cabc_moving_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->cabc.cabc_moving_cmds,
			lcd_kit_dbg.dbg_cabc_moving_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_bl_max(char *par)
{
	int value = 0;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_parse_u32_digit(par, &value, 1);
	if (dbg_ops->rgbw_bl_max)
		dbg_ops->rgbw_bl_max(value);
	LCD_KIT_INFO("value = %d\n", value);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_set_mode1(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_rgbw_mode1_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_rgbw_mode1_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_rgbw_mode1_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->rgbw_set_mode1 && len > 0)
		dbg_ops->rgbw_set_mode1(lcd_kit_dbg.dbg_rgbw_mode1_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_set_mode2(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_rgbw_mode2_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_rgbw_mode2_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_rgbw_mode2_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->rgbw_set_mode2 && len > 0)
		dbg_ops->rgbw_set_mode2(lcd_kit_dbg.dbg_rgbw_mode2_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_set_mode3(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_rgbw_mode3_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_rgbw_mode3_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_rgbw_mode3_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->rgbw_set_mode3 && len > 0)
		dbg_ops->rgbw_set_mode3(lcd_kit_dbg.dbg_rgbw_mode3_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_set_mode4(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_rgbw_mode4_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_rgbw_mode4_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_rgbw_mode4_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->rgbw_set_mode4 && len > 0)
		dbg_ops->rgbw_set_mode4(lcd_kit_dbg.dbg_rgbw_mode4_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_backlight_cmd(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_rgbw_backlight_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_rgbw_backlight_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_rgbw_backlight_cmds, LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->rgbw_backlight_cmd && len > 0)
		dbg_ops->rgbw_backlight_cmd(lcd_kit_dbg.dbg_rgbw_backlight_cmds,
			len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_rgbw_pixel_gainlimit_cmd(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_rgbw_pixel_gainlimit_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_rgbw_pixel_gainlimit_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par,
		lcd_kit_dbg.dbg_rgbw_pixel_gainlimit_cmds, LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->rgbw_pixel_gainlimit_cmd && len > 0)
		dbg_ops->rgbw_pixel_gainlimit_cmd(lcd_kit_dbg.dbg_rgbw_pixel_gainlimit_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_esd_reg_cmd(char *par)
{
	int len;

	lcd_kit_dbg.dbg_esd_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_esd_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_esd_cmds, LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->esd.cmds,
			lcd_kit_dbg.dbg_esd_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_esd_value(char *par)
{
	int i;
	int cnt;
	unsigned int value[MAX_REG_READ_COUNT] = {0};

	cnt = lcd_kit_parse_u32_digit(par, value, MAX_REG_READ_COUNT);
	LCD_KIT_INFO("cnt = %d\n", cnt);
	common_info->esd.value.cnt = cnt;
	for (i = 0; i < common_info->esd.value.cnt; i++) {
		if (common_info->esd.value.buf)
			common_info->esd.value.buf[i] = value[i];
	}
	return 0;
}

static int lcd_kit_dbg_dirty_region_cmd(char *par)
{
	int len;

	lcd_kit_dbg.dbg_dirty_region_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_dirty_region_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_dirty_region_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&common_info->dirty_region.cmds,
			lcd_kit_dbg.dbg_dirty_region_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_barcode_2d_cmd(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_barcode_2d_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_barcode_2d_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_barcode_2d_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->barcode_2d_cmd && len > 0)
		dbg_ops->barcode_2d_cmd(lcd_kit_dbg.dbg_barcode_2d_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_brightness_color_cmd(char *par)
{
	int len;
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg.dbg_brightness_color_cmds = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!lcd_kit_dbg.dbg_brightness_color_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(par, lcd_kit_dbg.dbg_brightness_color_cmds,
		LCD_KIT_CMD_MAX_NUM);
	if (dbg_ops->brightness_color_cmd && len > 0)
		dbg_ops->brightness_color_cmd(lcd_kit_dbg.dbg_brightness_color_cmds, len);
	return LCD_KIT_OK;
}

static int lcd_kit_dbg_vci_voltage(char *par)
{
	int i;
	int cnt;
	unsigned int value[VALUE_MAX] = {0};
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	cnt = lcd_kit_parse_u32_digit(par, value, VALUE_MAX);
	LCD_KIT_INFO("cnt = %d\n", cnt);
	if (power_hdl->lcd_vci.buf) {
		for (i = 0; i < VALUE_MAX; i++) {
			power_hdl->lcd_vci.buf[i] = value[i];
			LCD_KIT_INFO("power_hdl->lcd_vci.buf[%d] = %d\n",
				i, power_hdl->lcd_vci.buf[i]);
		}
	}
	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		if (dbg_ops->set_voltage)
			dbg_ops->set_voltage();
	}
	return 0;
}

static int lcd_kit_dbg_iovcc_voltage(char *par)
{
	int i;
	int cnt;
	unsigned int value[VALUE_MAX] = {0};
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	cnt = lcd_kit_parse_u32_digit(par, value, VALUE_MAX);
	LCD_KIT_INFO("cnt = %d\n", cnt);
	if (power_hdl->lcd_iovcc.buf) {
		for (i = 0; i < VALUE_MAX; i++) {
			power_hdl->lcd_iovcc.buf[i] = value[i];
			LCD_KIT_INFO("power_hdl->lcd_iovcc.buf[%d] = %d\n",
				i, power_hdl->lcd_iovcc.buf[i]);
		}
	}
	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		if (dbg_ops->set_voltage)
			dbg_ops->set_voltage();
	}
	return 0;
}

static int lcd_kit_dbg_vdd_voltage(char *par)
{
	int i;
	int cnt;
	unsigned int value[VALUE_MAX] = {0};
	struct lcd_kit_dbg_ops *dbg_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	cnt = lcd_kit_parse_u32_digit(par, value, VALUE_MAX);
	LCD_KIT_INFO("cnt = %d\n", cnt);
	if (power_hdl->lcd_vdd.buf) {
		for (i = 0; i < VALUE_MAX; i++) {
			power_hdl->lcd_vdd.buf[i] = value[i];
			LCD_KIT_INFO("power_hdl->lcd_vdd.buf[%d] = %d\n",
				i, power_hdl->lcd_vdd.buf[i]);
		}
	}
	if (power_hdl->lcd_vdd.buf &&
		power_hdl->lcd_vdd.buf[POWER_MODE] == REGULATOR_MODE) {
		if (dbg_ops->set_voltage)
			dbg_ops->set_voltage();
	}
	return 0;
}

static int lcd_kit_dbg_vsp_voltage(char *par)
{
	int i;
	int cnt;
	unsigned int value[VALUE_MAX] = {0};
	struct lcd_kit_dbg_ops *dbg_ops = NULL;
	struct lcd_kit_bias_ops *bias_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("bias_ops is null\n");
		return LCD_KIT_FAIL;
	}
	cnt = lcd_kit_parse_u32_digit(par, value, VALUE_MAX);
	LCD_KIT_INFO("cnt = %d\n", cnt);
	if (power_hdl->lcd_vsp.buf) {
		for (i = 0; i < VALUE_MAX; i++) {
			power_hdl->lcd_vsp.buf[i] = value[i];
			LCD_KIT_INFO("power_hdl->lcd_vsp.buf[%d] = %d\n",
				i, power_hdl->lcd_vsp.buf[i]);
		}
	}
	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE) {
		if (dbg_ops->set_voltage)
			dbg_ops->set_voltage();
	} else if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == GPIO_MODE) {
		if (bias_ops->dbg_set_bias_voltage)
			bias_ops->dbg_set_bias_voltage(
				power_hdl->lcd_vsp.buf[POWER_VOL],
				power_hdl->lcd_vsn.buf[POWER_VOL]);
	}
	return 0;
}

static int lcd_kit_dbg_vsn_voltage(char *par)
{
	int i;
	int cnt;
	unsigned int value[VALUE_MAX] = {0};
	struct lcd_kit_dbg_ops *dbg_ops = NULL;
	struct lcd_kit_bias_ops *bias_ops = NULL;

	dbg_ops = lcd_kit_get_debug_ops();
	if (!dbg_ops) {
		LCD_KIT_ERR("dbg_ops is null\n");
		return LCD_KIT_FAIL;
	}
	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("bias_ops is null\n");
		return LCD_KIT_FAIL;
	}
	cnt = lcd_kit_parse_u32_digit(par, value, VALUE_MAX);
	LCD_KIT_INFO("cnt = %d\n", cnt);
	if (power_hdl->lcd_vsn.buf) {
		for (i = 0; i < VALUE_MAX; i++) {
			power_hdl->lcd_vsn.buf[i] = value[i];
			LCD_KIT_INFO("power_hdl->lcd_vsn.buf[%d] = %d\n",
				i, power_hdl->lcd_vsn.buf[i]);
		}
	}
	if (power_hdl->lcd_vsn.buf &&
		power_hdl->lcd_vsn.buf[POWER_MODE] == REGULATOR_MODE) {
		if (dbg_ops->set_voltage)
			dbg_ops->set_voltage();
	} else if (power_hdl->lcd_vsn.buf &&
		power_hdl->lcd_vsn.buf[POWER_MODE] == GPIO_MODE) {
		if (bias_ops->dbg_set_bias_voltage)
			bias_ops->dbg_set_bias_voltage(
				power_hdl->lcd_vsp.buf[POWER_VOL],
				power_hdl->lcd_vsn.buf[POWER_VOL]);
	}
	return 0;
}

static void lcd_kit_dbg_set_dbg_level(int level)
{
	lcd_kit_msg_level = level;
}

int *lcd_kit_dbg_find_item(unsigned char *item)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(item_func); i++) {
		if (!strncmp(item, item_func[i].name, strlen(item))) {
			LCD_KIT_INFO("found %s\n", item);
			return (int *)item_func[i].func;
		}
	}
	LCD_KIT_ERR("not found %s\n", item);
	return NULL;
}

DBG_FUNC func;
static int parse_status = PARSE_HEAD;
static int cnt;
static int count;

static int right_angle_bra_pro(unsigned char *item_name,
	unsigned char *tmp_name, unsigned char *data)
{
	int ret;

	if (!item_name || !tmp_name || !data) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	if (parse_status == PARSE_HEAD) {
		func = (DBG_FUNC)lcd_kit_dbg_find_item(item_name);
		cnt = 0;
		parse_status = RECEIVE_DATA;
	} else if (parse_status == PARSE_FINAL) {
		if (strncmp(item_name, tmp_name, strlen(item_name))) {
			LCD_KIT_ERR("item head match final\n");
			return LCD_KIT_FAIL;
		}
		if (func) {
			LCD_KIT_INFO("data:%s\n", data);
			ret = func(data);
			if (ret)
				LCD_KIT_ERR("func execute err:%d\n", ret);
		}
		/* parse new item start */
		parse_status = PARSE_HEAD;
		count = 0;
		cnt = 0;
		memset(data, 0, LCD_KIT_CONFIG_TABLE_MAX_NUM);
		memset(item_name, 0, LCD_KIT_ITEM_NAME_MAX);
		memset(tmp_name, 0, LCD_KIT_ITEM_NAME_MAX);
	}
	return LCD_KIT_OK;
}

static int parse_ch(unsigned char *ch, unsigned char *data,
	unsigned char *item_name, unsigned char *tmp_name)
{
	int ret;

	switch (*ch) {
	case '<':
		if (parse_status == PARSE_HEAD)
			parse_status = PARSE_HEAD;
		return LCD_KIT_OK;
	case '>':
		ret = right_angle_bra_pro(item_name, tmp_name, data);
		if (ret < 0) {
			LCD_KIT_ERR("right_angle_bra_pro error\n");
			return LCD_KIT_FAIL;
		}
		return LCD_KIT_OK;
	case '/':
		if (parse_status == RECEIVE_DATA)
			parse_status = PARSE_FINAL;
		return LCD_KIT_OK;
	default:
		if (parse_status == PARSE_HEAD && is_valid_char(*ch)) {
			if (cnt >= LCD_KIT_ITEM_NAME_MAX) {
				LCD_KIT_ERR("item is too long\n");
				return LCD_KIT_FAIL;
			}
			item_name[cnt++] = *ch;
			return LCD_KIT_OK;
		}
		if (parse_status == PARSE_FINAL && is_valid_char(*ch)) {
			if (cnt >= LCD_KIT_ITEM_NAME_MAX) {
				LCD_KIT_ERR("item is too long\n");
				return LCD_KIT_FAIL;
			}
			tmp_name[cnt++] = *ch;
			return LCD_KIT_OK;
		}
		if (parse_status == RECEIVE_DATA) {
			if (count >= LCD_KIT_CONFIG_TABLE_MAX_NUM) {
				LCD_KIT_ERR("data is too long\n");
				return LCD_KIT_FAIL;
			}
			data[count++] = *ch;
			return LCD_KIT_OK;
		}
	}
	return LCD_KIT_OK;
}

static int parse_fd(const int *fd, unsigned char *data,
	unsigned char *item_name, unsigned char *tmp_name)
{
	unsigned char ch = '\0';
	int ret;
	int cur_seek = 0;

	if (!fd || !data || !item_name || !tmp_name) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	while (1) {
		if ((unsigned int)sys_read(*fd, &ch, 1) != 1) {
			LCD_KIT_INFO("it's end of file\n");
			break;
		}
		cur_seek++;
		ret = sys_lseek(*fd, (off_t)cur_seek, 0);
		if (ret < 0)
			LCD_KIT_ERR("sys_lseek error!\n");
		ret = parse_ch(&ch, data, item_name, tmp_name);
		if (ret < 0) {
			LCD_KIT_ERR("parse_ch error!\n");
			return LCD_KIT_FAIL;
		}
		continue;
	}
	return LCD_KIT_OK;
}

int lcd_kit_dbg_parse_config(void)
{
	unsigned char *item_name = NULL;
	unsigned char *tmp_name = NULL;
	unsigned char *data = NULL;
	int fd;
	mm_segment_t fs;
	int ret;

	fs = get_fs(); /* save previous value */
	set_fs(get_ds()); /* use kernel limit */
	fd = sys_open((const char __force *) LCD_KIT_PARAM_FILE_PATH, O_RDONLY, 0);
	if (fd < 0) {
		LCD_KIT_ERR("%s file doesn't exsit\n", LCD_KIT_PARAM_FILE_PATH);
		set_fs(fs);
		return FALSE;
	}
	LCD_KIT_INFO("Config file %s opened\n", LCD_KIT_PARAM_FILE_PATH);
	ret = sys_lseek(fd, (off_t)0, 0);
	if (ret < 0)
		LCD_KIT_ERR("sys_lseek error!\n");
	data = kzalloc(LCD_KIT_CONFIG_TABLE_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	item_name = kzalloc(LCD_KIT_ITEM_NAME_MAX, 0);
	if (!item_name) {
		kfree(data);
		LCD_KIT_ERR("item_name kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	tmp_name = kzalloc(LCD_KIT_ITEM_NAME_MAX, 0);
	if (!tmp_name) {
		kfree(data);
		kfree(item_name);
		LCD_KIT_ERR("tmp_name kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_fd(&fd, data, item_name, tmp_name);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		sys_close(fd);
		set_fs(fs);
		kfree(data);
		kfree(item_name);
		kfree(tmp_name);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("parse success\n");
	sys_close(fd);
	set_fs(fs);
	kfree(data);
	kfree(item_name);
	kfree(tmp_name);
	return LCD_KIT_OK;
}

/* open function */
static int lcd_kit_dbg_open(struct inode *inode, struct file *file)
{
	/* non-seekable */
	file->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE);
	return 0;
}

/* release function */
static int lcd_kit_dbg_release(struct inode *inode, struct file *file)
{
	return 0;
}

/* read function */
static ssize_t lcd_kit_dbg_read(struct file *file,  char __user *buff,
	size_t count, loff_t *ppos)
{
	int len;
	int ret_len;
	char *cur = NULL;
	int buf_len = sizeof(lcd_kit_debug_buf);

	cur = lcd_kit_debug_buf;
	if (*ppos)
		return 0;
	/* show usage */
	len = snprintf(cur, buf_len, "Usage:\n");
	buf_len -= len;
	cur += len;

	len = snprintf(cur, buf_len, "\teg. echo set_debug_level:4 > /sys/kernel/debug/lcd-dbg/lcd_kit_dbg to open set debug level\n");
	buf_len -= len;
	cur += len;

	len = snprintf(cur, buf_len, "\teg. echo set_param_config:1 > /sys/kernel/debug/lcd-dbg/lcd_kit_dbg to set parameter\n");
	buf_len -= len;
	cur += len;

	ret_len = sizeof(lcd_kit_debug_buf) - buf_len;

	// error happened!
	if (ret_len < 0)
		return 0;

	/* copy to user */
	if (copy_to_user(buff, lcd_kit_debug_buf, ret_len))
		return -EFAULT;

	*ppos += ret_len; // increase offset
	return ret_len;
}

/* write function */
static ssize_t lcd_kit_dbg_write(struct file *file, const char __user *buff,
	size_t count, loff_t *ppos)
{
#define BUF_LEN 256
	char *cur = NULL;
	int ret = 0;
	int cmd_type = -1;
	int cnt = 0;
	int i;
	int val;
	char lcd_debug_buf[BUF_LEN];

	cur = lcd_debug_buf;
	if (count > (BUF_LEN - 1))
		return count;
	if (copy_from_user(lcd_debug_buf, buff, count))
		return -EFAULT;
	lcd_debug_buf[count] = 0;
	/* convert to lower case */
	if (lcd_kit_str_to_lower(cur) != 0)
		goto err_handle;
	LCD_KIT_DEBUG("cur=%s, count=%d!\n", cur, (int)count);
	/* get cmd type */
	for (i = 0; i < ARRAY_SIZE(lcd_kit_cmd_list); i++) {
		if (!lcd_kit_str_start_with(cur, lcd_kit_cmd_list[i].pstr)) {
			cmd_type = lcd_kit_cmd_list[i].type;
			cur += strlen(lcd_kit_cmd_list[i].pstr);
			break;
		}
		LCD_KIT_DEBUG("lcd_kit_cmd_list[%d].pstr=%s\n", i,
			lcd_kit_cmd_list[i].pstr);
	}
	if (i >= ARRAY_SIZE(lcd_kit_cmd_list)) {
		LCD_KIT_ERR("cmd type not find!\n");  // not support
		goto err_handle;
	}
	switch (cmd_type) {
	case LCD_KIT_DBG_LEVEL_SET:
		cnt = sscanf(cur, ":%d", &val);
		if (cnt != 1) {
			LCD_KIT_ERR("get param fail!\n");
			goto err_handle;
		}
		lcd_kit_dbg_set_dbg_level(val);
		break;
	case LCD_KIT_DBG_PARAM_CONFIG:
		cnt = sscanf(cur, ":%d", &val);
		if (cnt != 1) {
			LCD_KIT_ERR("get param fail!\n");
			goto err_handle;
		}
		lcd_kit_dbg_free();
		if (val == 1) {
			ret = lcd_kit_dbg_parse_config();
			if (!ret)
				LCD_KIT_INFO("parse parameter succ!\n");
		}
		break;
	default:
		LCD_KIT_ERR("cmd type not support!\n"); // not support
		ret = LCD_KIT_FAIL;
		break;
	}
	/* finish */
	if (ret) {
		LCD_KIT_ERR("fail\n");
		goto err_handle;
	} else {
		return count;
	}

err_handle:
	return -EFAULT;
}

static const struct file_operations lcd_kit_debug_fops = {
	.open = lcd_kit_dbg_open,
	.release = lcd_kit_dbg_release,
	.read = lcd_kit_dbg_read,
	.write = lcd_kit_dbg_write,
};

/* init lcd debugfs interface */
int lcd_kit_debugfs_init(void)
{
	static char already_init;  // internal flag
	struct dentry *dent = NULL;
	struct dentry *file = NULL;

	/* judge if already init */
	if (already_init) {
		LCD_KIT_ERR("(%d): already init\n", __LINE__);
		return LCD_KIT_OK;
	}
	/* create dir */
	dent = debugfs_create_dir("lcd-dbg", NULL);
	if (IS_ERR_OR_NULL(dent)) {
		LCD_KIT_ERR("(%d):create_dir fail, error %ld\n", __LINE__,
			PTR_ERR(dent));
		dent = NULL;
		goto err_create_dir;
	}
	/* create reg_dbg_mipi node */
	file = debugfs_create_file("lcd_kit_dbg", 0644, dent, 0,
		&lcd_kit_debug_fops);
	if (IS_ERR_OR_NULL(file)) {
		LCD_KIT_ERR("(%d):create_file: lcd_kit_dbg fail\n", __LINE__);
		goto err_create_mipi;
	}
	already_init = 1;  // set flag
	return LCD_KIT_OK;
err_create_mipi:
		debugfs_remove_recursive(dent);
err_create_dir:
	return LCD_KIT_FAIL;
}

/*
 * lcd dpd
 */
int file_sys_is_ready(void)
{
	int fd = -1;
	int ret;
	struct device_node *np = NULL;
	char *lcd_name = NULL;
	char xml_name[XML_FILE_MAX];

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n",
			"huawei,lcd_panel_type");
		return LCD_KIT_FAIL;
	}
	lcd_name = (char *)of_get_property(np, "lcd_panel_type", NULL);
	if (!lcd_name) {
		LCD_KIT_ERR("can not get lcd kit compatible\n");
		return LCD_KIT_FAIL;
	}
	memset(xml_name, 0, sizeof(xml_name));
	ret = snprintf(xml_name, XML_FILE_MAX, "/odm/lcd/%s.xml", lcd_name);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error: %s\n", xml_name);
		return LCD_KIT_FAIL;
	}
	fd = sys_access(xml_name, 0);
	if (fd) {
		LCD_KIT_ERR("can not access file %d %s!\n", fd, xml_name);
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

struct delayed_work detect_fs_work;
int g_dpd_enter;
void detect_fs_wq_handler(struct work_struct *work)
{
	if (file_sys_is_ready()) {
		LCD_KIT_INFO("sysfs is not ready!\n");
		device_unblock_probing();
		/* delay 500ms schedule work */
		schedule_delayed_work(&detect_fs_work, msecs_to_jiffies(500));
		return;
	}
	LCD_KIT_INFO("sysfs is ready!\n");
	g_dpd_enter = 1; // lcd dpd is ok, entry lcd dpd mode
	device_unblock_probing();
}

/* previous attribute name in xml file */
unsigned char item_name[LCD_KIT_ITEM_NAME_MAX];
/* attribute name char count */
static int item_cnt;
/* attribute data char count */
static int data_cnt;
/* parse xml file status varial */
static int par_status = PARSE_HEAD;
static int receive_data(int status, const char *ch, char *out, int len)
{
	switch (status) {
	case PARSE_HEAD:
		if (is_valid_char(*ch)) {
			if (item_cnt >= LCD_KIT_ITEM_NAME_MAX) {
				LCD_KIT_ERR("item is too long\n");
				return LCD_KIT_FAIL;
			}
			item_name[item_cnt++] = *ch;
		}
		break;
	case RECEIVE_DATA:
		if (data_cnt >= len) {
			LCD_KIT_ERR("data is too long\n");
			return LCD_KIT_FAIL;
		}
		out[data_cnt++] = *ch;
		break;
	case NOT_MATCH:
	case PARSE_FINAL:
	case INVALID_DATA:
		break;
	default:
		break;
	}
	return LCD_KIT_OK;
}

static int parse_char(const char *item, const char *ch,
	char *out, int len, int *find)
{
	int item_len;
	int tmp_len;

	switch (*ch) {
	case '<':
		if (par_status == RECEIVE_DATA) {
			*find = 1;
			return LCD_KIT_OK;
		}
		par_status = PARSE_HEAD;
		data_cnt = 0;
		item_cnt = 0;
		memset(item_name, 0, LCD_KIT_ITEM_NAME_MAX);
		break;
	case '>':
		if (par_status == PARSE_HEAD) {
			tmp_len = strlen(item_name);
			item_len = strlen(item);
			if ((tmp_len != item_len) ||
				strncmp(item_name, item, tmp_len)) {
				par_status = NOT_MATCH;
				return LCD_KIT_OK;
			}
			item_cnt = 0;
			par_status = RECEIVE_DATA;
		}
		break;
	case '/':
		par_status = PARSE_FINAL;
		break;
	case '!':
		if (par_status == PARSE_HEAD)
			par_status = INVALID_DATA;
		break;
	default:
		receive_data(par_status, ch, out, len);
		break;
	}
	return LCD_KIT_OK;
}

static int parse_open(const char *path)
{
	int ret;
	int fd = -1;

	set_fs(get_ds()); //lint !e501
	fd = sys_open((const char __force *)path, O_RDONLY, 0);
	if (fd < 0) {
		LCD_KIT_ERR("%s file doesn't exsit\n", path);
		return LCD_KIT_FAIL;
	}
	ret = sys_lseek(fd, (off_t)0, 0);
	if (ret < 0) {
		LCD_KIT_ERR("sys_lseek error!\n");
		sys_close(fd);
		return LCD_KIT_FAIL;
	}
	return fd;
}

static int parse_xml(const char *path, const char *item, char *out, int len)
{
	char ch = '\0';
	int ret;
	int cur_seek = 0;
	int find = 0;
	int fd = -1;
	mm_segment_t fs;

	if (!out) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	fs = get_fs(); /* save previous value */
	fd = parse_open(path);
	if (fd < 0) {
		LCD_KIT_ERR("open file:%s fail!\n", path);
		set_fs(fs);
		return LCD_KIT_FAIL;
	}
	par_status = PARSE_HEAD;
	while (1) {
		/* read 1 char from file */
		if ((unsigned int)sys_read(fd, &ch, 1) != 1) {
			LCD_KIT_INFO("it's end of file\n");
			break;
		}
		cur_seek++;
		ret = sys_lseek(fd, (off_t)cur_seek, 0);
		if (ret < 0) {
			LCD_KIT_ERR("sys_lseek error!\n");
			break;
		}
		ret = parse_char(item, &ch, out, len, &find);
		if (ret < 0) {
			LCD_KIT_ERR("parse ch error!\n");
			break;
		}
		if (find) {
			LCD_KIT_INFO("find item:%s = %s\n", item, out);
			break;
		}
		continue;
	}
	if (find)
		ret = LCD_KIT_OK;
	else
		ret = LCD_KIT_FAIL;
	set_fs(fs);
	sys_close(fd);
	return ret;
}

static int stat_value_count(const char *in)
{
	char ch;
	int cnt_num = 1;
	int len;
	int i = 0;

	if (!in) {
		LCD_KIT_ERR("in or out is null\n");
		return LCD_KIT_FAIL;
	}
	len = strlen(in);
	while (len--) {
		ch = in[i++];
		if (ch != ',' && ch != '\n')
			continue;
		cnt_num++; // calculate data number separate use ', '
	}
	return cnt_num;
}

static char *str_convert(const char *item)
{
	int i;
	int len;
	int tmp_len;
	const int array_size = ARRAY_SIZE(xml_string_name_map);

	tmp_len = strlen(item);
	for (i = 0; i < array_size; i++) {
		len = strlen(xml_string_name_map[i].dtsi_string_name);
		if (tmp_len != len)
			continue;
		if (!strncmp(item,
			xml_string_name_map[i].dtsi_string_name, len))
			return xml_string_name_map[i].xml_string_name;
	}
	LCD_KIT_INFO("not find item:%s\n", item);
	return NULL;
}

static int parse_u32(const char *path, const char *item,
	unsigned int *out, unsigned int def)
{
	int ret;
	char *data = NULL;
	unsigned int value = 0;
	/*
	 * request (0xffffffff->4294967295) max 10 byte to save
	 * string and 1 byte to save '\0'
	 */
	const int u32_str_size = 11;

	data = kzalloc(u32_str_size, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	*out = def;
	ret = parse_xml(path, item, data, u32_str_size);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	/* parse 1 digit */
	lcd_kit_parse_u32_digit(data, &value, 1);
	*out = value;
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_u8(const char *path, const char *item,
	unsigned char *out, unsigned char def)
{
	int ret;
	char *data = NULL;
	unsigned int value = 0;

	data = kzalloc(sizeof(unsigned int), 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	*out = def;
	ret = parse_xml(path, item, data, sizeof(unsigned int));
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	/* parse 1 digit */
	lcd_kit_parse_u32_digit(data, &value, 1);
	*out = (unsigned char)(value & 0xff);
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_dcs(const char *path, const char *item,
	const char *item_state, struct lcd_kit_dsi_panel_cmds *pcmds)
{
	int ret;
	int len;
	char *data = NULL;
	char *temp_buff = NULL; // can not free, it will be used

	data = kzalloc(LCD_KIT_DCS_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_DCS_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	len = stat_value_count(data);
	if (len <= 0) {
		LCD_KIT_ERR("invalid len\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	temp_buff = kzalloc(sizeof(int) * (len + 1), 0);
	if (!temp_buff) {
		LCD_KIT_ERR("kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	len = lcd_kit_parse_u8_digit(data, temp_buff, len);
	if (len <= 0) {
		LCD_KIT_ERR("lcd_kit_parse_u8_digit error\n");
		kfree(temp_buff);
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_cmd(pcmds, temp_buff, len);
	ret = parse_u32(path, item_state, &(pcmds->link_state),
		LCD_KIT_DSI_HS_MODE);
	if (ret < 0)
		LCD_KIT_INFO("parse link_state fail\n");
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_arrays(const char *path, const char *item,
	struct lcd_kit_arrays_data *out, int size)
{
	char ch = '"';
	char *data = NULL;
	unsigned int *temp_buff = NULL; // can not free, it will be used
	int ret;
	int len;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = stat_value_count(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return LCD_KIT_FAIL;
	}
	temp_buff = kzalloc(sizeof(int) * (len + 1), 0);
	if (!temp_buff) {
		LCD_KIT_ERR("temp_buff kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	if (size <= 0) {
		kfree(temp_buff);
		kfree(data);
		LCD_KIT_ERR("size is error\n");
		return LCD_KIT_FAIL;
	}
	len = len / size;
	if (len <= 0) {
		kfree(temp_buff);
		kfree(data);
		LCD_KIT_ERR("len is error\n");
		return LCD_KIT_FAIL;
	}
	out->arry_data = kzalloc(sizeof(struct lcd_kit_array_data) * len, 0);
	if (!out->arry_data) {
		LCD_KIT_ERR("out->arry_data kzalloc fail\n");
		kfree(temp_buff);
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_dbg_parse_array(data, temp_buff, out, size);
	kfree(data);
	return LCD_KIT_OK;
}

static int parse_array(const char *path, const char *item,
	struct lcd_kit_array_data *out)
{
	char ch = '"';
	char *data = NULL;
	unsigned int *temp_buff = NULL; // can not free, it will be used
	int ret;
	int len;
	int cnt_num;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = stat_value_count(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return LCD_KIT_FAIL;
	}
	temp_buff = kzalloc(sizeof(int) * (len + 1), 0);
	if (!temp_buff) {
		LCD_KIT_ERR("temp_buff kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	cnt_num = lcd_kit_parse_u32_digit(data, temp_buff, len);
	out->buf = temp_buff;
	out->cnt = cnt_num;
	kfree(data);
	return LCD_KIT_OK;
}

static char *parse_string(const char *path, const char *item)
{
	char ch = '"';
	char *data = NULL;
	char *string = NULL;
	int ret;
	int len;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return NULL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return NULL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = strlen(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return NULL;
	}
	string = kzalloc((len + 1), 0);
	if (!string) {
		LCD_KIT_ERR("string kzalloc fail\n");
		kfree(data);
		return NULL;
	}
	strncpy(string, data, len);
	kfree(data);
	return string;
}

static int parse_string_array(const char *path, const char *item,
	const char **out)
{
	char ch = '"';
	char *data = NULL;
	char *string = NULL;
	char *str1 = NULL;
	char *str2 = NULL;
	int ret;
	int len;

	data = kzalloc(LCD_KIT_CMD_MAX_NUM, 0);
	if (!data) {
		LCD_KIT_ERR("data kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	ret = parse_xml(path, item, data, LCD_KIT_CMD_MAX_NUM);
	if (ret < 0) {
		LCD_KIT_INFO("parse fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	lcd_kit_str_to_del_ch(data, ch);
	len = strlen(data);
	if (len <= 0) {
		kfree(data);
		LCD_KIT_ERR("invalid len\n");
		return LCD_KIT_FAIL;
	}
	string = kzalloc((len + 1), 0);
	if (!string) {
		LCD_KIT_ERR("temp_buff kzalloc fail\n");
		kfree(data);
		return LCD_KIT_FAIL;
	}
	strncpy(string, data, len);
	str1 = string;
	do {
		str2 = strstr(str1, ",");
		if (str2 == NULL) {
			*out++ = str1;
			break;
		}
		*str2 = 0;
		*out++ = str1;
		str2++;
		str1 = str2;
	} while (str2 != NULL);
	kfree(data);
	return LCD_KIT_OK;
}

static void parse_dirty_region(const char *path, const char *item,
	int *out)
{
	int value = 0;
	int ret;

	ret = parse_u32(path, item, &value, 0xffff);
	if (ret)
		LCD_KIT_INFO("parse item:%s fail\n", item);
	if (value == 0xffff)
		value = -1;
	*out = value;
}

int g_dpd_mode;
static int __init early_parse_debug_cmdline(char *arg)
{
	char debug[CMDLINE_MAX + 1];
	int ret;

	if (!arg) {
		LCD_KIT_ERR("arg is NULL\n");
		return 0;
	}
	memset(debug, 0, (CMDLINE_MAX + 1));
	memcpy(debug, arg, CMDLINE_MAX);
	debug[CMDLINE_MAX] = '\0';
	ret = kstrtouint(debug, 10, &g_dpd_mode);
	if (ret)
		return ret;
	LCD_KIT_INFO("g_dpd_mode = %d\n", g_dpd_mode);
	return 0;
}
early_param("lcd_dpd", early_parse_debug_cmdline);

char g_xml_name[XML_NAME_MAX];
static char *dpd_get_xml_name(const struct device_node *np)
{
	char *comp = NULL;
	int ret;

	comp = (char *)of_get_property(np, "compatible", NULL);
	if (!comp) {
		LCD_KIT_ERR("comp is null\n");
		return NULL;
	}
	memset(g_xml_name, 0, sizeof(g_xml_name));
	ret = snprintf(g_xml_name, XML_NAME_MAX, "/odm/lcd/%s.xml", comp);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf fail\n");
		return NULL;
	}
	return g_xml_name;
}

int lcd_parse_u32(const struct device_node *np, const char *item,
	unsigned int *out, unsigned int def)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("np or item or out is null\n");
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_u32(path, item_tmp, out, def);
}

int lcd_parse_u8(const struct device_node *np, const char *item,
	unsigned char *out, unsigned char def)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("np or item or out is null\n");
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_u8(path, item_tmp, out, def);
}

int lcd_parse_dcs(const struct device_node *np, const char *item,
	const char *item_state, struct lcd_kit_dsi_panel_cmds *pcmds)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !item_state || !pcmds) {
		LCD_KIT_ERR("np or item or out is null\n");
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_dcs(path, item_tmp, item_state, pcmds);
}

int lcd_parse_arrays(const struct device_node *np, const char *item,
	struct lcd_kit_arrays_data *out, int size)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("np or item or out is null\n");
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_arrays(path, item_tmp, out, size);
}

int lcd_parse_array(const struct device_node *np, const char *item,
	struct lcd_kit_array_data *out)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("np or item or out is null\n");
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_array(path, item_tmp, out);
}

char *lcd_parse_string(const struct device_node *np, const char *item)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item) {
		LCD_KIT_ERR("np or item or out is null\n");
		return NULL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return NULL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return NULL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	return parse_string(path, item_tmp);
}


int lcd_parse_string_array(const struct device_node *np, const char *item,
	const char **out)
{
	char *path = NULL;
	char *item_tmp = NULL;
	int ret;

	if (!np || !item || !out) {
		LCD_KIT_ERR("np or item or out is null\n");
		return LCD_KIT_FAIL;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return LCD_KIT_FAIL;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	ret = parse_string_array(path, item_tmp, out);
	if (ret) {
		LCD_KIT_ERR("parse string array fail\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

void lcd_parse_dirty_region(const struct device_node *np, const char *item,
	int *out)
{
	char *path = NULL;
	char *item_tmp = NULL;

	if (!np || !item || !out) {
		LCD_KIT_ERR("np or item or out is null\n");
		return;
	}
	path = dpd_get_xml_name(np);
	if (!path) {
		LCD_KIT_ERR("path is null\n");
		return;
	}
	item_tmp = str_convert(item);
	if (!item_tmp) {
		LCD_KIT_ERR("string convert fail\n");
		return;
	}
	LCD_KIT_INFO("item_tmp = %s\n", item_tmp);
	LCD_KIT_INFO("path = %s\n", path);
	parse_dirty_region(path, item_tmp, out);
}

int is_dpd_mode(void)
{
	if ((g_dpd_mode == 1) && (g_dpd_enter == 1))
		return 1; // dpd mode
	return 0; // normal mode
}

