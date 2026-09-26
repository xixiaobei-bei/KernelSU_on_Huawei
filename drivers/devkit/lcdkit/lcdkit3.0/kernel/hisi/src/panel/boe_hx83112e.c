/*
 * boe_hx83112e.c
 *
 * boe_hx83112e lcd driver
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

static int boe_hx112e_rgbw_set(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_dsi_panel_cmds *rgbw_cmds)
{
	uint32_t sat_ctl = (uint32_t)hisifd->de_info.rgbw_saturation_control;
	uint32_t frm_gain = (uint32_t)hisifd->de_info.frame_gain_limit;
	uint32_t clr = (uint32_t)hisifd->de_info.color_distortion_allowance;
	uint32_t pxl_gain = (uint32_t)hisifd->de_info.pixel_gain_limit;
	uint32_t pwm_duty = (uint32_t)hisifd->de_info.pwm_duty_gain;
	uint32_t rgbw_total = (uint32_t)hisifd->de_info.rgbw_total_glim;
	static uint32_t sat_ctl_old;
	static uint32_t frm_gain_old;
	static uint32_t clr_old;
	static uint32_t pxl_gain_old;
	static uint32_t pwm_duty_old;
	static uint32_t rgbw_total_old;
	int ret = LCD_KIT_OK;

	if ((!(rgbw_cmds->cmds)) || (rgbw_cmds->cmd_cnt <= 0)) {
		LCD_KIT_ERR("rgbw_cmds->cmds is null or cmd_cnt <=0\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("[RGBW] rgbw_mode=%d!\n", hisifd->de_info.ddic_rgbw_mode);
	if ((sat_ctl_old != sat_ctl) || (frm_gain_old != frm_gain) ||
		(clr_old != clr) || (pxl_gain_old != pxl_gain) ||
		(pwm_duty_old != pwm_duty) || (rgbw_total_old != rgbw_total)) {
		// pwm_duty_gain, 0x50 means base value
		rgbw_cmds->cmds[2].payload[1] = 0x50 + ((pwm_duty >> 8) & 0x03);
		rgbw_cmds->cmds[2].payload[2] = pwm_duty & 0xff;
		LCD_KIT_DEBUG("[RGBW] pwm_duty_gain=%d pwm_duty_gain_old=%d!\n",
			pwm_duty, pwm_duty_old);
		// pixel_gain_limit, 0x80 means base value
		rgbw_cmds->cmds[2].payload[6] = 0x80 + ((pxl_gain >> 8) & 0x01);
		rgbw_cmds->cmds[2].payload[7] = pxl_gain & 0xff;
		LCD_KIT_DEBUG("[RGBW] pixel_gain=%d pixel_gain_old = %d!\n",
			pxl_gain, pxl_gain_old);
		// frame_gain_limit, 0x02 means base value
		rgbw_cmds->cmds[2].payload[8] = 0x02 + ((frm_gain >> 8) & 0x01);
		rgbw_cmds->cmds[2].payload[9] = frm_gain & 0xff;
		LCD_KIT_DEBUG("[RGBW] frame_gain=%d frame_gain_old = %d!\n",
			frm_gain, frm_gain_old);
		// color_distortion_allowance
		rgbw_cmds->cmds[2].payload[13] = clr & 0xff;
		LCD_KIT_DEBUG("[RGBW] clr=%d clr = %d!\n", clr, clr_old);
		// rgbw total glim, 0x3E means base value
		rgbw_cmds->cmds[2].payload[14] = 0x3E + ((rgbw_total >> 8) & 0x01);
		rgbw_cmds->cmds[2].payload[15] = rgbw_total & 0xff;
		LCD_KIT_DEBUG("[RGBW] rgbw_total=%d rgbw_total_old = %d!\n",
			rgbw_total, rgbw_total_old);
		// rgbw_saturation_control setting, 0x40 means base value
		rgbw_cmds->cmds[2].payload[18] = 0x40 + ((sat_ctl >> 8) & 0x03);
		rgbw_cmds->cmds[2].payload[19] = sat_ctl & 0xff;
		LCD_KIT_DEBUG("[RGBW] sat_ctl=%d, sat_ctl_old=%d!\n",
			sat_ctl, sat_ctl_old);
		ret = lcd_kit_dsi_cmds_tx(hisifd, rgbw_cmds);
	}
	sat_ctl_old = sat_ctl;
	frm_gain_old = frm_gain;
	clr_old = clr;
	pxl_gain_old = pxl_gain;
	pwm_duty_old = pwm_duty;
	rgbw_total_old = rgbw_total;
	return ret;
}

static int boe_hx83112e_rgbw(struct hisi_fb_data_type *hisifd, int mode)
{
	int rgbw_mode;
	int ret;

	rgbw_mode = hisifd->de_info.ddic_rgbw_mode;
	switch (rgbw_mode) {
	case RGBW_SET1_MODE:
		ret = boe_hx112e_rgbw_set(hisifd, &disp_info->rgbw.mode1_cmds);
		break;
	case RGBW_SET2_MODE:
		ret = boe_hx112e_rgbw_set(hisifd, &disp_info->rgbw.mode2_cmds);
		break;
	case RGBW_SET3_MODE:
		ret = boe_hx112e_rgbw_set(hisifd, &disp_info->rgbw.mode3_cmds);
		break;
	case RGBW_SET4_MODE:
		ret = boe_hx112e_rgbw_set(hisifd, &disp_info->rgbw.mode4_cmds);
		break;
	default:
		ret = LCD_KIT_FAIL;
		break;
	}
	return ret;
}


static struct lcd_kit_panel_ops boe_hx83112e_ops = {
	.lcd_kit_rgbw_set_mode = boe_hx83112e_rgbw,
};

int boe_hx83112e_probe(void)
{
	int ret;

	ret = lcd_kit_panel_ops_register(&boe_hx83112e_ops);
	if (ret) {
		LCD_KIT_ERR("failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
