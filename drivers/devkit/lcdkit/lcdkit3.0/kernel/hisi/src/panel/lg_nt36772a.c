/*
 * lg_nt36772a.c
 *
 * lg_nt36772a lcd driver
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

static int lg_nt36772a_rgbw_set(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_dsi_panel_cmds *rgbw_cmds)
{
	uint32_t rgbw_ctl = (uint32_t)hisifd->de_info.rgbw_saturation_control;
	uint32_t frm_gain = (uint32_t)hisifd->de_info.frame_gain_limit;
	uint32_t clr = (uint32_t)hisifd->de_info.color_distortion_allowance;
	uint32_t pxl_gain = (uint32_t)hisifd->de_info.pixel_gain_limit;
	uint32_t pwm_duty = (uint32_t)hisifd->de_info.pwm_duty_gain;
	static uint32_t rgbw_ctl_old;
	static uint32_t frm_gain_old;
	static uint32_t clr_old;
	static uint32_t pxl_gain_old;
	static uint32_t pwm_duty_old;
	int ret = LCD_KIT_OK;

	if ((!(rgbw_cmds->cmds)) || (rgbw_cmds->cmd_cnt <= 0)) {
		LCD_KIT_ERR("rgbw_cmds->cmds is null or cmd_cnt <=0\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("[RGBW] rgbw_mode=%d!\n", hisifd->de_info.ddic_rgbw_mode);
	if ((rgbw_ctl_old != rgbw_ctl) || (frm_gain_old != frm_gain) ||
		(clr_old != clr) || (pxl_gain_old != pxl_gain) ||
		(pwm_duty_old != pwm_duty)) {
		// rgbw_saturation_control setting
		rgbw_cmds->cmds[1].payload[1] = (rgbw_ctl >> 24) & 0xff;
		rgbw_cmds->cmds[1].payload[2] = (rgbw_ctl >> 16) & 0xff;
		rgbw_cmds->cmds[1].payload[3] = (rgbw_ctl >> 8) & 0xff;
		rgbw_cmds->cmds[1].payload[4] = rgbw_ctl & 0xff;
		LCD_KIT_DEBUG("[RGBW] rgbw_ctl=%d, rgbw_ctl_old = %d!\n",
			rgbw_ctl, rgbw_ctl_old);
		// frame_gain_limit
		rgbw_cmds->cmds[1].payload[5] = (frm_gain >> 8) & 0x3f;
		rgbw_cmds->cmds[1].payload[6] = frm_gain & 0xff;
		LCD_KIT_DEBUG("[RGBW] frm_gain=%d frm_gain_old = %d!\n",
			frm_gain, frm_gain_old);
		// color_distortion_allowance
		rgbw_cmds->cmds[1].payload[9] = (clr >> 8) & 0x3f;
		rgbw_cmds->cmds[1].payload[10] = clr & 0xff;
		LCD_KIT_DEBUG("[RGBW] clr=%d clr_old = %d!\n", clr, clr_old);
		// pixel_gain_limit
		rgbw_cmds->cmds[1].payload[11] = (pxl_gain >> 8) & 0x3f;
		rgbw_cmds->cmds[1].payload[12] = pxl_gain & 0xff;
		LCD_KIT_DEBUG("[RGBW] pxl_gain=%d pxl_gain_old = %d!\n",
			pxl_gain, pxl_gain_old);
		// pwm_duty_gain
		rgbw_cmds->cmds[1].payload[15] = (pwm_duty >> 8) & 0x3f;
		rgbw_cmds->cmds[1].payload[16] = pwm_duty & 0xff;
		LCD_KIT_DEBUG("[RGBW] pwm_duty=%d pwm_duty_old = %d!\n",
			pwm_duty, pwm_duty_old);
		ret = lcd_kit_dsi_cmds_tx(hisifd, rgbw_cmds);
	}
	rgbw_ctl_old = rgbw_ctl;
	frm_gain_old = frm_gain;
	clr_old = clr;
	pxl_gain_old = pxl_gain;
	pwm_duty_old = pwm_duty;
	return ret;
}

static int lg_nt36772a_rgbw(struct hisi_fb_data_type *hisifd, int mode)
{
	int rgbw_mode;
	int ret;

	rgbw_mode = hisifd->de_info.ddic_rgbw_mode;
	switch (rgbw_mode) {
	case RGBW_SET1_MODE:
		ret = lg_nt36772a_rgbw_set(hisifd, &disp_info->rgbw.mode1_cmds);
		break;
	case RGBW_SET2_MODE:
		ret = lg_nt36772a_rgbw_set(hisifd, &disp_info->rgbw.mode2_cmds);
		break;
	case RGBW_SET3_MODE:
		ret = lg_nt36772a_rgbw_set(hisifd, &disp_info->rgbw.mode3_cmds);
		break;
	case RGBW_SET4_MODE:
		ret = lg_nt36772a_rgbw_set(hisifd, &disp_info->rgbw.mode4_cmds);
		break;
	default:
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}


static struct lcd_kit_panel_ops lg_nt36772a_ops = {
	.lcd_kit_rgbw_set_mode = lg_nt36772a_rgbw,
};

int lg_nt36772a_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&lg_nt36772a_ops);
	if (ret) {
		LCD_KIT_ERR("failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
