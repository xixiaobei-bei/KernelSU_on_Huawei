/*
 * lcd_kit_disp.c
 *
 * lcdkit display function for lcd driver
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

#include "lcd_kit_disp.h"
#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_dbg.h"
#include <huawei_platform/log/log_jank.h>
#include "global_ddr_map.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_power.h"
#include "bias_bl_utils.h"
#ifdef LV_GET_LCDBK_ON
#include <huawei_platform/inputhub/sensor_feima_ext.h>
#endif
#ifdef LCD_FACTORY_MODE
#include "lcd_kit_factory.h"
#endif
#include <linux/mfd/hisi_pmic.h>

static const unsigned int fps_90hz = 90;
static const uint32_t te_interval_60_fps_us = 16667;
static const uint64_t te_interval_90_fps_us = 11111;

static int lcd_kit_set_fastboot(struct platform_device *pdev);
static int lcd_kit_on(struct platform_device *pdev);
static int lcd_kit_off(struct platform_device *pdev);
static int lcd_kit_remove(struct platform_device *pdev);
static int lcd_kit_set_backlight(struct platform_device *pdev, uint32_t bl_level);
static int lcd_kit_set_backlight_by_type(struct platform_device *pdev, int backlight_type);
static int lcd_kit_set_hbm_for_screenon(struct platform_device *pdev, int bl_type);
static int lcd_kit_set_hbm_for_mmi(struct platform_device *pdev, int level);
static int lcd_kit_esd_check(struct platform_device *pdev);
static int lcd_kit_set_display_region(struct platform_device *pdev, struct dss_rect *dirty);
static int lcd_kit_fps_scence_handle(struct platform_device *pdev, uint32_t scence);
static int lcd_kit_fps_updt_handle(struct platform_device *pdev);
static int lcd_kit_set_tcon_mode(struct platform_device *pdev, uint8_t mode);
static ssize_t lcd_kit_ce_mode_store(struct platform_device *pdev, const char *buf, size_t count);
static ssize_t lcd_kit_rgbw_set_func(struct hisi_fb_data_type *hisifd);
static ssize_t lcd_kit_hbm_set_func(struct hisi_fb_data_type *hisifd);
static ssize_t lcd_kit_set_ic_dim_on(struct hisi_fb_data_type *hisifd);
static ssize_t lcd_kit_cabc_store(struct platform_device *pdev, const char *buf, size_t count);
static ssize_t lcd_kit_color_param_get_func(struct hisi_fb_data_type *hisifd);
static int lcd_get_demura_func(struct platform_device *pdev,
	unsigned char dsi, unsigned char *out,
	unsigned char type, unsigned char len);
static int lcd_set_demura_func(struct platform_device *pdev,
	unsigned char type, const demura_set_info_t *info);
/* variable declare */
static struct timer_list backlight_second_timer;
static struct lcd_kit_disp_info g_lcd_kit_disp_info;
static enum pcd_check_status g_pcd_check_status = PCD_CHECK_WAIT;
#ifdef LV_GET_LCDBK_ON
static const u32 g_mipi_level = 1417; // for UD sensor when HBM is on
#endif

struct lcd_panel_name_map g_panel_name_map[] = {
	{ "samsung_ea8079_taurus", "taurus_802_079" },
	{ "sm_s6e3ha8_lion", "lion_808_J01" },
	{ "lg_nt37280_lion", "lion_874_280" },
	{ "boe_r66458_6p53", "lion_808_458" }
};

static struct hisi_panel_info lcd_kit_pinfo = {0};
static struct hisi_fb_panel_data lcd_kit_data = {
	.panel_info = &lcd_kit_pinfo,
	.set_fastboot = lcd_kit_set_fastboot,
	.on = lcd_kit_on,
	.off = lcd_kit_off,
	.remove = lcd_kit_remove,
	.set_backlight = lcd_kit_set_backlight,
	.lcd_set_backlight_by_type_func = lcd_kit_set_backlight_by_type,
	.lcd_set_hbm_for_screenon = lcd_kit_set_hbm_for_screenon,
	.lcd_set_hbm_for_mmi_func = lcd_kit_set_hbm_for_mmi,
	.esd_handle = lcd_kit_esd_check,
	.set_display_region = lcd_kit_set_display_region,
	.lcd_fps_scence_handle = lcd_kit_fps_scence_handle,
	.lcd_fps_updt_handle = lcd_kit_fps_updt_handle,
	.lcd_ce_mode_store = lcd_kit_ce_mode_store,
	.lcd_rgbw_set_func = lcd_kit_rgbw_set_func,
	.lcd_hbm_set_func  = lcd_kit_hbm_set_func,
	.lcd_set_ic_dim_on  = lcd_kit_set_ic_dim_on,
	.lcd_color_param_get_func = lcd_kit_color_param_get_func,
	.lcd_cabc_mode_store = lcd_kit_cabc_store,
#ifdef LCD_FACTORY_MODE
	.panel_bypass_powerdown_ulps_support = lcd_kit_get_pt_ulps_support,
#endif
	.set_tcon_mode = lcd_kit_set_tcon_mode,
	.lcd_get_demura = lcd_get_demura_func,
	.lcd_set_demura = lcd_set_demura_func,
};

#if defined CONFIG_HUAWEI_DSM
struct dsm_client *lcd_kit_get_lcd_dsm_client(void)
{
	return lcd_dclient;
}
#endif

struct lcd_kit_disp_info *lcd_kit_get_disp_info(void)
{
	return &g_lcd_kit_disp_info;
}
static int uc_panel_is_power_on;
static int lcd_kit_panel_is_power_on(struct hisi_fb_data_type *hisifd)
{
	uint32_t temp;
	int ret;
	char __iomem *sctrl_base = NULL;
	/*
	 * bit[8] = 1 : lcd power on
	 * bit[8] = 0 : lcd power off
	 */
	sctrl_base = hisifd->sctrl_base;
	temp = inp32(sctrl_base + SCBAKDATA11);
	ret  = (temp & 0x100) >> 8; // it means get bit[8] of temp

	LCD_KIT_INFO("inp32(SOC_SCTRL_SCBAKDATA11_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))= 0x%x bit[8] = %d!\n",
		temp, ret);
	return ret;
}
static void lcd_kit_clear_sctrl_reg(struct hisi_fb_data_type *hisifd)
{
	uint32_t temp;
	char __iomem *sctrl_base = NULL;

	/*
	 * bit[8] = 1 : lcd power on
	 * bit[8] = 0 : lcd power off
	 */
	sctrl_base = hisifd->sctrl_base;
	temp = inp32(sctrl_base + SCBAKDATA11);
	temp &= ~(0x100);
	outp32(sctrl_base + SCBAKDATA11, temp);
	LCD_KIT_INFO("outp32(SOC_SCTRL_SCBAKDATA11_ADDR(SOC_ACPU_SCTRL_BASE_ADDR), 0x%x)\n",
		temp);
}

void lcd_kit_set_power_status(int status)
{
	uc_panel_is_power_on = status;
}

int lcd_kit_get_power_status(void)
{
	return uc_panel_is_power_on;
}

static int lcd_kit_set_elvss_dim_lp(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (hisifd == NULL) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.hbm_set_elvss_dim_lp) {
		LCD_KIT_INFO("Do not support enable ELVSS Dim in power on\n");
		return ret;
	}
	if (common_info->hbm.elvss_prepare_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.elvss_prepare_cmds);
		}
	}
	if (common_info->hbm.elvss_write_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			common_info->hbm.elvss_write_cmds.cmds[0].wait = LCD_KIT_ELVSSDIM_NO_WAIT;
			common_info->hbm.elvss_write_cmds.cmds[0].payload[1] =
				common_info->hbm.ori_elvss_val | LCD_KIT_ENABLE_ELVSSDIM_MASK;
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.elvss_write_cmds);
			LCD_KIT_INFO("[power on set elvss dim] send:0x%x\n",
				common_info->hbm.elvss_write_cmds.cmds[0].payload[1]);
		}
	}
	if (common_info->hbm.elvss_post_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.elvss_post_cmds);
	}
	return ret;
}

static void lcd_kit_update_gamma_from_tpic(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;
	static bool has_read;
	struct ts_kit_ops *ts_ops = NULL;

	if (!disp_info->otp_gamma.support) {
		LCD_KIT_INFO("not support otp gamma\n");
		return;
	}

	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return;
	}
	/* read gamma from tpic */
	if (!has_read) {
		has_read = true;
		/* ret:0--invalid gamma, 1--valid gamma */
		if (ts_ops->read_otp_gamma) {
			ret = ts_ops->read_otp_gamma(disp_info->otp_gamma.gamma,
				GAMMA_MAX);
			if (ret < 0) {
				LCD_KIT_ERR("read otp gamma error\n");
				return;
			}
		}
	}
	/* print gamma head and len */
	LCD_KIT_INFO("HEAD:0x%x, LEN:0x%x\n", disp_info->otp_gamma.gamma[0],
		disp_info->otp_gamma.gamma[1]);
	/* verify gamma */
	if ((disp_info->otp_gamma.gamma[0] != GAMMA_HEAD &&
		disp_info->otp_gamma.gamma[0] != GRAY_HEAD) ||
		(disp_info->otp_gamma.gamma[1] != GAMMA_LEN)) {
		LCD_KIT_INFO("not otp gamma\n");
		return;
	}
	if (disp_info->otp_gamma.gamma[0] == GAMMA_HEAD)
		lcd_kit_set_otp_gamma(hisifd);
	if (disp_info->otp_gamma.gamma[0] == GRAY_HEAD)
		lcd_kit_set_otp_gray(hisifd);
}

static void check_pcd_errflag_report(struct hisi_fb_data_type *hisifd)
{
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL\n");
		return;
	}
	(void)lcd_kit_check_pcd_errflag_check(hisifd);
}

static void lcd_kit_set_thp_proximity_sem(bool sem_lock)
{
	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("thp_proximity not support!\n");
		return;
	}
	if (sem_lock == true)
		down(&disp_info->thp_second_poweroff_sem);
	else
		up(&disp_info->thp_second_poweroff_sem);
}

static void lcd_kit_set_thp_proximity_state(int power_state)
{
	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("thp_proximity not support!\n");
		return;
	}
	common_info->thp_proximity.panel_power_state = power_state;
}

static void report_amoled_power_ic_check_err(int32_t gpio_value)
{
#if defined CONFIG_HUAWEI_DSM
	int8_t record_buf[DMD_RECORD_BUF_LEN] = {'\0'};
	int32_t recordtime = 0;
	int32_t ret;

	ret = snprintf(record_buf, DMD_RECORD_BUF_LEN,
		"amoled power ic status error : gpio[%u] expect value = %u, read value = %d\n",
		disp_info->amoled_power_ic_check.gpio, disp_info->amoled_power_ic_check.expect_value,
		gpio_value);
	if (ret < 0)
		LCD_KIT_ERR("snprintf happened error! return %d\n", ret);
	(void)lcd_dsm_client_record(lcd_dclient, record_buf,
		DSM_AMOLED_POWER_IC_CHECK_ERROR_NO,
		REC_DMD_NO_LIMIT,
		&recordtime);
#endif
}

static void lcd_kit_amoled_power_ic_check(void)
{
	int32_t ret;
	int32_t gpio_value;

	ret = gpio_request(disp_info->amoled_power_ic_check.gpio, "power_ic_check_gpio");
	if (ret != 0) {
		LCD_KIT_ERR("amoled_power_ic_check_gpio[%u] request fail! return %d\n",
			disp_info->amoled_power_ic_check.gpio, ret);
		return;
	}
	ret = gpio_direction_input(disp_info->amoled_power_ic_check.gpio);
	if (ret != 0) {
		gpio_free(disp_info->amoled_power_ic_check.gpio);
		LCD_KIT_ERR("amoled_power_ic_check_gpio[%u] direction set fail! return %d\n",
			disp_info->amoled_power_ic_check.gpio, ret);
		return;
	}
	gpio_value = gpio_get_value(disp_info->amoled_power_ic_check.gpio);
	gpio_free(disp_info->amoled_power_ic_check.gpio);

	if (gpio_value != disp_info->amoled_power_ic_check.expect_value) {
		LCD_KIT_ERR("amoled power ic status abnormal! check_gpio[%u], expect value = %u, read value = %d\n",
			disp_info->amoled_power_ic_check.gpio, disp_info->amoled_power_ic_check.expect_value,
			gpio_value);
		report_amoled_power_ic_check_err(gpio_value);
		return;
	}
	LCD_KIT_DEBUG("amoled power ic status normal! check_gpio[%u], expect value = %u, read value = %d\n",
		disp_info->amoled_power_ic_check.gpio, disp_info->amoled_power_ic_check.expect_value, gpio_value);
}

static void lcd_open_elvdd_detect(struct hisi_fb_data_type *hisifd)
{
	struct lcd_kit_adapt_ops *ops = NULL;
	static unsigned int cnt = 0;
	int ret = LCD_KIT_OK;

	if (!(common_info->elvdd_detect.support))
		return;
	ops = lcd_kit_get_adapt_ops();
	if (!ops) {
		LCD_KIT_ERR("can not register ops!\n");
		return;
	}
	if (!hisifd->panel_info.esd_enable) {
		LCD_KIT_INFO("[elvdd detect]esd closed\n");
		return;
	}
	/* Elvdd detect consecutive starts less than the threshold */
	if (hisifd->esd_recover_state != ESD_RECOVER_STATE_START)
		cnt = 0;
	if ((hisifd->esd_recover_state == ESD_RECOVER_STATE_START) &&
		(cnt < hisifd->panel_info.esd_recovery_max_count))
		cnt++;
	if (cnt >= hisifd->panel_info.esd_recovery_max_count) {
		LCD_KIT_ERR("[elvdd detect] beyond max count 0x%x!\n", cnt);
		return;
	}
	if (ops->mipi_tx)
		ret = ops->mipi_tx((void *)hisifd,
			&common_info->elvdd_detect.cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("[elvdd detect] mipi tx fail!\n");
}

static int32_t poweric_cmd_detect(struct hisi_fb_data_type *hisifd,
	uint8_t *out)
{
	uint8_t read_value = 0;
	int32_t ret;

	ret = lcd_kit_dsi_cmds_rx(hisifd, &read_value,
		&disp_info->elvdd_detect.cmds);
	if (ret) {
		LCD_KIT_ERR("mipi rx failed!\n");
		return LCD_KIT_OK;
	}
	if ((read_value & disp_info->elvdd_detect.exp_value_mask) !=
		disp_info->elvdd_detect.exp_value)
		ret = LCD_KIT_FAIL;
	*out = read_value;

	LCD_KIT_INFO("read_value = 0x%x, exp_value = 0x%x, mask = 0x%x\n",
		read_value,
		disp_info->elvdd_detect.exp_value,
		disp_info->elvdd_detect.exp_value_mask);
	return ret;
}

static int32_t poweric_gpio_detect(int32_t *out)
{
	int32_t gpio_value;
	int32_t ret;

	ret = gpio_request(disp_info->elvdd_detect.detect_gpio, "elvdd_gpio");
	if (ret != 0) {
		LCD_KIT_ERR("pcd_gpio[%d] request fail!\n",
			disp_info->elvdd_detect.detect_gpio);
		return LCD_KIT_OK;
	}
	ret = gpio_direction_input(disp_info->elvdd_detect.detect_gpio);
	if (ret != 0) {
		gpio_free(disp_info->elvdd_detect.detect_gpio);
		LCD_KIT_ERR("pcd_gpio[%d] direction set fail!\n",
			disp_info->elvdd_detect.detect_gpio);
		return LCD_KIT_OK;
	}
	gpio_value = gpio_get_value(disp_info->elvdd_detect.detect_gpio);
	gpio_free(disp_info->elvdd_detect.detect_gpio);

	if (gpio_value != disp_info->elvdd_detect.exp_value)
		ret = LCD_KIT_FAIL;
	*out = gpio_value;
	LCD_KIT_INFO("elvdd_gpio_value = %d\n", gpio_value);
	return ret;
}

static void report_poweric_err(uint8_t cmd_val, int32_t gpio_val)
{
#if defined CONFIG_HUAWEI_DSM
	int8_t record_buf[DMD_RECORD_BUF_LEN] = {'\0'};
	int32_t recordtime = 0;
	int32_t ret;

	ret = snprintf(record_buf, DMD_RECORD_BUF_LEN,
		"elvdd: detect_type = 0x%x, cmd_val = 0x%x, gpio_val = %d\n",
		disp_info->elvdd_detect.detect_type, cmd_val, gpio_val);
	if (ret < 0)
		LCD_KIT_ERR("snprintf happened error!\n");
	(void)lcd_dsm_client_record(lcd_dclient, record_buf,
		DSM_LCD_STATUS_ERROR_NO,
		REC_DMD_NO_LIMIT,
		&recordtime);
#endif
}

static void lcd_kit_poweric_detect(struct hisi_fb_data_type *hisifd)
{
	static int32_t retry_times;
	int32_t ret = 0;
	uint8_t cmd_val = 0;
	int32_t gpio_val = 0;

	if (!disp_info->elvdd_detect.support)
		return;
	if (disp_info->elvdd_detect.detect_type == ELVDD_MIPI_CHECK_MODE)
		ret = poweric_cmd_detect(hisifd, &cmd_val);
	else if (disp_info->elvdd_detect.detect_type == ELVDD_GPIO_CHECK_MODE)
		ret = poweric_gpio_detect(&gpio_val);
	if (ret) {
		LCD_KIT_ERR("detect poweric abnomal, recovery lcd\n");
		report_poweric_err(cmd_val, gpio_val);
		if (retry_times >= RECOVERY_TIMES) {
			LCD_KIT_WARNING("not need recovery, recovery num:%d\n",
				retry_times);
			retry_times = 0;
			return;
		}
		hisifd->esd_recover_state = ESD_RECOVER_STATE_START;
		if (hisifd->esd_ctrl.esd_check_wq)
			queue_work(hisifd->esd_ctrl.esd_check_wq,
				&(hisifd->esd_ctrl.esd_check_work));
		retry_times++;
		return;
	}
	retry_times = 0;
	LCD_KIT_INFO("detect poweric nomal\n");
}
static const char *lcd_kit_read_panel_name(const char *panel_name)
{
	int i;
	int panel_num = ARRAY_SIZE(g_panel_name_map);

	for (i = 0; i < panel_num; i++) {
		if (!strcmp(g_panel_name_map[i].panel_name, panel_name))
			return g_panel_name_map[i].hide_panel_name;
	}
	return panel_name;
}
static int lcd_kit_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;
	int ret = LCD_KIT_OK;
	const char *panel_name = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->aod_function) {
		LCD_KIT_INFO("AOD mode, bypass disp_kit_panel_on!\n");
		return LCD_KIT_OK;
	}
	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null!\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("fb%d, +!\n", hisifd->index);
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return LCD_KIT_FAIL;
	}
	switch (pinfo->lcd_init_step) {
	case LCD_INIT_POWER_ON:
		lcd_kit_set_thp_proximity_sem(true);
		lcd_kit_set_thp_proximity_state(POWER_ON);
		lcd_kit_set_power_status(lcd_kit_panel_is_power_on(hisifd));
		if (common_ops->panel_power_on)
			ret = common_ops->panel_power_on((void *)hisifd);
		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
		panel_name = common_info->panel_model != NULL ?
			common_info->panel_model : disp_info->compatible;
		panel_name = lcd_kit_read_panel_name(panel_name);
		LCD_KIT_INFO("lcd_name is %s\n", panel_name);
		lcd_kit_set_thp_proximity_sem(false);
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
		break;
	case LCD_INIT_MIPI_LP_SEND_SEQUENCE:
		/* send mipi command by low power */
		if (common_ops->panel_on_lp)
			ret = common_ops->panel_on_lp((void *)hisifd);
		lcd_open_elvdd_detect(hisifd);
		/* update gamma */
		lcd_kit_update_gamma_from_tpic(hisifd);
		lcd_kit_set_elvss_dim_lp(hisifd);
		pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
		break;
	case LCD_INIT_MIPI_HS_SEND_SEQUENCE:
		/* send mipi command by high speed */
		if (common_ops->panel_on_hs)
			ret = common_ops->panel_on_hs((void *)hisifd);
		/* record panel on time */
		lcd_kit_disp_on_record_time();
		if (lcd_kit_get_power_status()) {
			lcd_kit_clear_sctrl_reg(hisifd);
			lcd_kit_set_power_status(0);
		}
		if (lcd_ops->power_monitor_on)
			lcd_ops->power_monitor_on();
		/* let panel pcd start check,only at first power on */
		if (g_pcd_check_status == PCD_CHECK_WAIT) {
			(void)lcd_kit_start_pcd_check(hisifd);
			g_pcd_check_status = PCD_CHECK_ON;
		}
		break;
	case LCD_INIT_NONE:
		break;
	case LCD_INIT_LDI_SEND_SEQUENCE:
		break;
	default:
		break;
	}
	// backlight on
	hisi_lcd_backlight_on(pdev);
	LCD_KIT_INFO("fb%d, -!\n", hisifd->index);
	return ret;
}

static void lcd_kit_remove_shield_backlight(void)
{
	if (disp_info->bl_is_shield_backlight == true)
		disp_info->bl_is_shield_backlight = false;
	if (false != disp_info->bl_is_start_second_timer) {
		del_timer(&backlight_second_timer);
		disp_info->bl_is_start_second_timer = false;
		LCD_KIT_INFO("panel powerOff, clear backlight shield timer\n");
	}
}

static void lcd_kit_resume_hisi_panel_info(struct hisi_panel_info *pinfo)
{
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return;
	}
	if (disp_info->fps.support)
		pinfo->fps_updt = LCD_FPS_60;
}

/*
 * name:lcd_kit_off
 * function:power off panel
 * @pdev:platform device
 */
static int lcd_kit_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;
	char *panel_name = (common_info->panel_model != NULL) ?
		common_info->panel_model : disp_info->compatible;

	if (pdev == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->aod_function) {
		LCD_KIT_INFO("AOD mode, bypass disp_kit_panel_off!\n");
		return LCD_KIT_OK;
	}
	if (common_info->hbm.support)
		common_info->hbm.hbm_level_current = 0;
	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null!\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_remove_shield_backlight();
	LCD_KIT_INFO("fb%d, +!\n", hisifd->index);
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_set_thp_proximity_sem(true);
	switch (pinfo->lcd_uninit_step) {
	case LCD_UNINIT_MIPI_HS_SEND_SEQUENCE:
		/* check pcd errflag and report dmd */
		if (g_pcd_check_status == PCD_CHECK_ON) {
			check_pcd_errflag_report(hisifd);
			g_pcd_check_status = PCD_CHECK_OFF;
		}
		/* check amoled power ic errflag and report dmd */
		if (disp_info->amoled_power_ic_check.support)
			lcd_kit_amoled_power_ic_check();
		// check mipi errors
		common_ops->mipi_check(hisifd, panel_name,
			disp_info->quickly_sleep_out.panel_on_record_tv.tv_sec);
		// backlight off
		hisi_lcd_backlight_off(pdev);
		if (common_ops->panel_off_hs)
			common_ops->panel_off_hs(hisifd);
		if (lcd_ops->power_monitor_off)
			lcd_ops->power_monitor_off();
		pinfo->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
		lcd_kit_set_thp_proximity_state(POWER_TS_SUSPEND);
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "LCD_POWER_OFF");
		break;
	case LCD_UNINIT_MIPI_LP_SEND_SEQUENCE:
		if (common_ops->panel_off_lp)
			common_ops->panel_off_lp(hisifd);
		pinfo->lcd_uninit_step = LCD_UNINIT_POWER_OFF;
		break;
	case LCD_UNINIT_POWER_OFF:
		lcd_kit_set_thp_proximity_state(POWER_OFF);
		if (common_ops->panel_power_off)
			common_ops->panel_power_off(hisifd);
		lcd_kit_resume_hisi_panel_info(pinfo);
		break;
	default:
		break;
	}
	lcd_kit_set_thp_proximity_sem(false);
	LCD_KIT_INFO("fb%d, -!\n", hisifd->index);
	return LCD_KIT_OK;
}

/*
 * name:lcd_kit_remove
 * function:panel remove
 * @pdev:platform device
 */
static int lcd_kit_remove(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (!pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL Point!\n");
		return LCD_KIT_OK;
	}
	lcd_kit_power_finit(pdev);
	return LCD_KIT_OK;
}

static void lcd_kit_second_timerout_function(unsigned long arg)
{
	unsigned long temp;

	temp = arg;
	if (disp_info->bl_is_shield_backlight == true)
		disp_info->bl_is_shield_backlight = false;
	del_timer(&backlight_second_timer);
	disp_info->bl_is_start_second_timer = false;
	LCD_KIT_INFO("Sheild backlight 1.2s timeout, remove the bl sheild\n");
}

static void disable_elvss_dim_write(void)
{
	common_info->hbm.elvss_write_cmds.cmds[0].wait =
		common_info->hbm.hbm_fp_elvss_cmd_delay;
	common_info->hbm.elvss_write_cmds.cmds[0].payload[1] =
		common_info->hbm.ori_elvss_val & LCD_KIT_DISABLE_ELVSSDIM_MASK;
}

static void enable_elvss_dim_write(void)
{
	common_info->hbm.elvss_write_cmds.cmds[0].wait = LCD_KIT_ELVSSDIM_NO_WAIT;
	common_info->hbm.elvss_write_cmds.cmds[0].payload[1] =
		common_info->hbm.ori_elvss_val | LCD_KIT_ENABLE_ELVSSDIM_MASK;
}

static int lcd_kit_set_elvss_dim_fp(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_adapt_ops *adapt_ops, int disable_elvss_dim)
{
	int ret = LCD_KIT_OK;

	if ((hisifd == NULL) || (adapt_ops == NULL)) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.hbm_fp_elvss_support) {
		LCD_KIT_INFO("Do not support ELVSS Dim, just return\n");
		return ret;
	}
	if (common_info->hbm.elvss_prepare_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.elvss_prepare_cmds);
		}
	}
	if (common_info->hbm.elvss_write_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx) {
			if (disable_elvss_dim)
				disable_elvss_dim_write();
			else
				enable_elvss_dim_write();
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.elvss_write_cmds);
			LCD_KIT_INFO("[fp set elvss dim] send:0x%x\n",
				common_info->hbm.elvss_write_cmds.cmds[0].payload[1]);
		}
	}
	if (common_info->hbm.elvss_post_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.elvss_post_cmds);
	}
	return ret;
}

static int lcd_kit_enter_hbm_fb(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_adapt_ops *adapt_ops)
{
	int ret = LCD_KIT_OK;
	struct hisi_panel_info *pinfo = NULL;

	if ((hisifd == NULL) || (adapt_ops == NULL)) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return LCD_KIT_FAIL;
	}
	/* enable hbm */
	if ((pinfo->fps == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(hisifd,
			&common_info->dfr_info.cmds[FPS_90_HBM_NO_DIM]);
	} else if (common_info->hbm.fp_enter_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.fp_enter_cmds);
	}
	return ret;
}

static int lcd_kit_set_hbm_level_fp(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_adapt_ops *adapt_ops, unsigned int level)
{
	int ret = LCD_KIT_OK;

	if ((hisifd == NULL) || (adapt_ops == NULL)) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}

	/* prepare */
	if (common_info->hbm.hbm_prepare_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.hbm_prepare_cmds);
	}
	/* set hbm level */
	if (common_info->hbm.hbm_cmds.cmds != NULL) {
		if (common_info->hbm.hbm_special_bit_ctrl == LCD_KIT_HIGH_12BIT_CTL_HBM_SUPPORT) {
			/* Set high 12bit hbm level, low 4bit set 0 */
			common_info->hbm.hbm_cmds.cmds[0].payload[1] =
				(level >> LCD_KIT_SHIFT_FOUR_BIT) & 0xff;
			common_info->hbm.hbm_cmds.cmds[0].payload[2] =
				(level << LCD_KIT_SHIFT_FOUR_BIT) & 0xf0;
		} else if (common_info->hbm.hbm_special_bit_ctrl ==
					LCD_KIT_8BIT_CTL_HBM_SUPPORT) {
			common_info->hbm.hbm_cmds.cmds[0].payload[1] = level & 0xff;
		} else {
			/* change bl level to dsi cmds */
			common_info->hbm.hbm_cmds.cmds[0].payload[1] =
				(level >> 8) & 0xf;
			common_info->hbm.hbm_cmds.cmds[0].payload[2] =
				level & 0xff;
		}
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.hbm_cmds);
	}
	/* post */
	if (common_info->hbm.hbm_post_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.hbm_post_cmds);
	}

	return ret;
}

static int lcd_kit_disable_hbm_fp(struct hisi_fb_data_type *hisifd,
	struct lcd_kit_adapt_ops *adapt_ops)
{
	int ret = LCD_KIT_OK;
	struct hisi_panel_info *pinfo = NULL;

	if ((hisifd == NULL) || (adapt_ops == NULL)) {
		LCD_KIT_ERR("param is NULL!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return LCD_KIT_FAIL;
	}
	if ((pinfo->fps == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(hisifd,
			&common_info->dfr_info.cmds[FPS_90_NORMAL_NO_DIM]);
	} else if (common_info->hbm.exit_dim_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.exit_dim_cmds);
	}
	if (common_info->hbm.exit_cmds.cmds != NULL) {
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&common_info->hbm.exit_cmds);
	}

	return ret;
}

static ssize_t lcd_kit_fp_hbm_extern(struct hisi_fb_data_type *hisifd, int type)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;


	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	switch (type) {
	case LCD_KIT_FP_HBM_ENTER:
		if (common_ops->fp_hbm_enter_extern)
			ret = common_ops->fp_hbm_enter_extern(hisifd);
		break;
	case LCD_KIT_FP_HBM_EXIT:
		if (common_ops->fp_hbm_exit_extern)
			ret = common_ops->fp_hbm_exit_extern(hisifd);
		break;
	default:
		LCD_KIT_ERR("unknown case!\n");
		break;
	}
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_fp_hbm_extern fail!\n");
	}
	return ret;
}

static ssize_t lcd_kit_hbm_set_func_by_level(struct hisi_fb_data_type *hisifd,
	uint32_t level, int type)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return LCD_KIT_FAIL;
	}

	switch (type) {
	case LCD_KIT_FP_HBM_ENTER:
		if (lcd_kit_set_elvss_dim_fp(hisifd, adapt_ops, LCD_KIT_DISABLE_ELVSSDIM) < 0)
			LCD_KIT_ERR("set_elvss_dim_fp: disable failed!\n");
		if (lcd_kit_enter_hbm_fb(hisifd, adapt_ops) < 0)
			LCD_KIT_ERR("enter_hbm_fb: enable hbm failed!\n");
		if (lcd_kit_set_hbm_level_fp(hisifd, adapt_ops, level) < 0)
			LCD_KIT_ERR("set_hbm_level_fp: set level failed!\n");
		break;
	case LCD_KIT_FP_HBM_EXIT:
		if (level > 0) {
			if (lcd_kit_set_hbm_level_fp(hisifd, adapt_ops, level) < 0)
				LCD_KIT_ERR("set_hbm_fp: set level failed!\n");
		} else {
			if (lcd_kit_disable_hbm_fp(hisifd, adapt_ops) < 0)
				LCD_KIT_ERR("disable hbm failed!\n");
		}
		if (lcd_kit_set_elvss_dim_fp(hisifd, adapt_ops, LCD_KIT_ENABLE_ELVSSDIM) < 0)
			LCD_KIT_ERR("set_elvss_dim_fp: enable failed!\n");
		break;
	default:
		LCD_KIT_ERR("unknown case!\n");
		break;
	}
	return ret;
}

static int lcd_kit_restore_hbm_level(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is NULL!\n");
		return ret;
	}

	mutex_lock(&common_info->hbm.hbm_lock);
	ret = lcd_kit_hbm_set_func_by_level(hisifd,
		common_info->hbm.hbm_level_current, LCD_KIT_FP_HBM_EXIT);
	mutex_unlock(&common_info->hbm.hbm_lock);
	return ret;
}

static int lcd_kit_set_hbm_for_screenon(struct platform_device *pdev, int bl_type)
{
	int ret = 0;
	int max_backlight;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_kit_set_backlight_by_type) {
		ret = panel_ops->lcd_kit_set_backlight_by_type(pdev,
			bl_type, &backlight_second_timer);
		return ret;
	}
	if (pdev == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	LCD_KIT_INFO("backlight_type is %d\n", bl_type);
	max_backlight = hisifd->ud_fp_hbm_level;
	msleep(30);
	switch (bl_type) {
	case BACKLIGHT_HIGH_LEVEL:
		if (common_info->hbm.hbm_fp_support) {
			mutex_lock(&common_info->hbm.hbm_lock);
			common_info->hbm.hbm_if_fp_is_using = 1;
			lcd_kit_hbm_set_func_by_level(hisifd,
				common_info->hbm.hbm_level_max,
				LCD_KIT_FP_HBM_ENTER);
			mutex_unlock(&common_info->hbm.hbm_lock);
		} else {
			hisifd->panel_info.need_skip_delta = 1;
			(void)lcd_kit_mipi_set_backlight(hisifd, max_backlight);
			LCD_KIT_INFO("bl_type is (%d), set_bl is (%d)\n",
				bl_type, max_backlight);
		}
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V350)
		// To avoid XCC influencing the lightness
		hisifd->mask_layer_xcc_flag = 1;
		clear_xcc_table(hisifd);
#endif
		break;
	default:
		LCD_KIT_ERR("backlight_type is not define(%d)\n", bl_type);
		break;
	}
	return ret;
}

static unsigned long get_real_te_interval(struct hisi_panel_info *pinfo)
{
	if (pinfo == NULL) {
		HISI_FB_ERR("pinfo is NULL, use default te interval");
		return te_interval_60_fps_us;
	}
	if (pinfo->fps == fps_90hz) {
		HISI_FB_INFO("pinfo->fps == FPS_90HZ");
		return te_interval_90_fps_us;
	} else {
		return te_interval_60_fps_us;
	}
}

static unsigned long correct_time_based_on_fps(uint32_t real_te_interval,
	uint64_t time_60_fps)
{
	return time_60_fps * real_te_interval / te_interval_60_fps_us;
}

static void lcd_kit_fphbm_entry_delay(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	ktime_t cur_timestamp;
	unsigned long diff;
	unsigned long te_time;
	unsigned long delay_time;
	unsigned long delay_threshold;
	unsigned long real_te_interval;

	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL\n");
		return;
	}
	real_te_interval = get_real_te_interval(pinfo);
	delay_threshold = correct_time_based_on_fps(real_te_interval,
		pinfo->hbm_entry_delay);
	LCD_KIT_INFO("delay_threshold = %d us", delay_threshold);
	if (delay_threshold <= 0)
		return;
	cur_timestamp = ktime_get();
	diff = ktime_to_us(cur_timestamp) - ktime_to_us(pinfo->hbm_blcode_ts);
	te_time = correct_time_based_on_fps(real_te_interval,
		pinfo->te_interval_us);
	LCD_KIT_INFO("diff = %ld us, TE time = %ld us", diff, te_time);

	if (diff >= 0 && diff < delay_threshold) {
		delay_time = (delay_threshold - diff) / te_time * te_time + te_time;
		LCD_KIT_INFO("delay = %ld us", delay_time);
		udelay(delay_time);
	}
}

static int lcd_kit_set_backlight_by_type(struct platform_device *pdev, int backlight_type)
{
	int ret = 0;
	int max_backlight;
	int min_backlight;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_kit_set_backlight_by_type) {
		ret = panel_ops->lcd_kit_set_backlight_by_type(pdev,
			backlight_type, &backlight_second_timer);
		return ret;
	}
	if (!pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	LCD_KIT_INFO("backlight_type is %d\n", backlight_type);
	max_backlight = hisifd->ud_fp_hbm_level;
	min_backlight = hisifd->ud_fp_current_level;
	switch (backlight_type) {
	case BACKLIGHT_HIGH_LEVEL:
		if (common_info->hbm.hbm_fp_support) {
			mutex_lock(&common_info->hbm.hbm_lock);
			lcd_kit_fphbm_entry_delay(hisifd);
			common_info->hbm.hbm_if_fp_is_using = 1;
			if(!lcd_kit_is_current_frame_ok_to_set_fp_backlight(hisifd, BACKLIGHT_HIGH_LEVEL))
				usleep_range(te_interval_60_fps_us, te_interval_60_fps_us);
			lcd_kit_hbm_set_func_by_level(hisifd,
				common_info->hbm.hbm_level_max,
				LCD_KIT_FP_HBM_ENTER);
			mutex_unlock(&common_info->hbm.hbm_lock);
		} else {
			hisifd->panel_info.need_skip_delta = 1;
			(void)lcd_kit_set_backlight(pdev, max_backlight);
			lcd_kit_fp_hbm_extern(hisifd, LCD_KIT_FP_HBM_ENTER);
			hisifd->panel_info.need_skip_delta = 0;
		}
		disp_info->bl_is_shield_backlight = true;
		if (disp_info->bl_is_start_second_timer == false) {
			init_timer(&backlight_second_timer);
			backlight_second_timer.expires = jiffies + 12 * HZ / 10; // 1.2s
			backlight_second_timer.data = 0;
			backlight_second_timer.function = lcd_kit_second_timerout_function;
			add_timer(&backlight_second_timer);
			disp_info->bl_is_start_second_timer = true;
		} else {
			// if timer is not timeout, restart timer
			mod_timer(&backlight_second_timer, (jiffies + 12 * HZ / 10)); // 1.2s
		}
		LCD_KIT_INFO("bl_type is (%d), set_bl is (max_%d) : (pre_%d)\n",
			backlight_type, max_backlight, hisifd->bl_level);
		break;
	case BACKLIGHT_LOW_LEVEL:
		if (disp_info->bl_is_start_second_timer == true) {
			del_timer(&backlight_second_timer);
			disp_info->bl_is_start_second_timer = false;
		}
		disp_info->bl_is_shield_backlight = false;
		if (common_info->hbm.hbm_fp_support) {
			if(!lcd_kit_is_current_frame_ok_to_set_fp_backlight(hisifd, BACKLIGHT_LOW_LEVEL))
				usleep_range(te_interval_60_fps_us, te_interval_60_fps_us);
			lcd_kit_restore_hbm_level(hisifd);
			common_info->hbm.hbm_if_fp_is_using = 0;
		}
		lcd_kit_set_backlight(pdev, hisifd->bl_level);
		lcd_kit_fp_hbm_extern(hisifd, LCD_KIT_FP_HBM_EXIT);
		LCD_KIT_INFO("bl_type is (%d), set_bl is (pre_%d) : (cur_%d)\n",
			backlight_type, min_backlight, hisifd->bl_level);
		break;
	default:
		LCD_KIT_ERR("bl_type is not define(%d)\n", backlight_type);
		break;
	}
	return ret;
}

static int lcd_kit_set_hbm_for_mmi(struct platform_device *pdev, int level)
{
	int mmi_level, backlight_type;
	struct hisi_fb_data_type *hisifd = NULL;
	/* waiting fps max time */
	long timeout = msecs_to_jiffies(50);

	if (pdev == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	if (common_info->dfr_info.fps_lock_command_support) {
		wait_event_interruptible_timeout(
			common_info->dfr_info.hbm_hist,
			common_info->dfr_info.fps_dfr_status,
			timeout);
		common_info->dfr_info.hbm_status = HBM_STATUS_DOING;
		down(&hisifd->blank_sem);
		if (!hisifd->panel_power_on) {
			up(&hisifd->blank_sem);
			HISI_FB_DEBUG("panel power off!\n");
			return -EINVAL;
		}
		hisifb_activate_vsync(hisifd);
	}
	if (level == 0) {
		backlight_type = BACKLIGHT_LOW_LEVEL;
		mmi_level = hisifd->bl_level;
	} else {
		backlight_type = BACKLIGHT_HIGH_LEVEL;
		mmi_level = level;
	}
	switch (backlight_type) {
	case BACKLIGHT_HIGH_LEVEL:
		LCD_KIT_INFO("bl_type=%d, level=%d\n", backlight_type, level);
		if (common_info->hbm.hbm_fp_support) {
			lcd_kit_set_backlight(pdev, hisifd->panel_info.bl_max);
			mutex_lock(&common_info->hbm.hbm_lock);
			common_info->hbm.hbm_if_fp_is_using = 1;
			lcd_kit_hbm_set_func_by_level(hisifd, mmi_level,
				LCD_KIT_FP_HBM_ENTER);
			mutex_unlock(&common_info->hbm.hbm_lock);
		} else {
			hisifd->panel_info.need_skip_delta = 1;
			lcd_kit_set_backlight(pdev, mmi_level);
			lcd_kit_fp_hbm_extern(hisifd, LCD_KIT_FP_HBM_ENTER);
			hisifd->panel_info.need_skip_delta = 0;

		}
		disp_info->bl_is_shield_backlight = true;
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V510) || \
    defined(CONFIG_HISI_FB_V350)
		/*
		 * To avoid XCC influencing the lightness ,
		 * call the interface of mask layer matching
		 */
		hisifd->mask_layer_xcc_flag = 1;
		clear_xcc_table(hisifd);
#endif
		break;
	case BACKLIGHT_LOW_LEVEL:
		LCD_KIT_INFO("bl_type=%d  level=%d\n", backlight_type, level);
		disp_info->bl_is_shield_backlight = false;
		if (common_info->hbm.hbm_fp_support) {
			lcd_kit_restore_hbm_level(hisifd);
			mutex_lock(&common_info->hbm.hbm_lock);
			common_info->hbm.hbm_if_fp_is_using = 0;
			mutex_unlock(&common_info->hbm.hbm_lock);
		}
		lcd_kit_set_backlight(pdev, mmi_level);
		lcd_kit_fp_hbm_extern(hisifd, LCD_KIT_FP_HBM_EXIT);
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V510) || \
    defined(CONFIG_HISI_FB_V350)
		/*
		 * restore XCC config(include XCC enable state)
		 * while mmi test exit
		 */
		restore_xcc_table(hisifd);
		hisifd->mask_layer_xcc_flag = 0;
#endif
		break;
	default:
		LCD_KIT_ERR("bl_type is not define(%d)\n", backlight_type);
		break;
	}
	if (common_info->dfr_info.fps_lock_command_support) {
		common_info->dfr_info.hbm_status = HBM_STATUS_IDLE;
		wake_up_interruptible(&common_info->dfr_info.fps_hist);
		hisifb_deactivate_vsync(hisifd);
		up(&hisifd->blank_sem);
	}
	return LCD_KIT_OK;
}

static bool lcd_kit_first_screenon(uint32_t last_bl_level, uint32_t bl_level)
{
	bool ret = false;

	if (last_bl_level == 0 && bl_level != 0) {
		LCD_KIT_INFO("first_screenon, bl_level = %d", bl_level);
		ret = true;
	} else {
		ret = false;
	}
	last_bl_level = bl_level;
	return ret;
}

static int lcd_kit_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	static uint32_t jank_last_bl_level;
	static uint32_t bl_type;
	struct hisi_panel_info *pinfo = NULL;
	bool first_screenon = false;

	if (pdev == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->aod_mode && hisifd->aod_function) {
		LCD_KIT_INFO("It is in AOD mode and bypass lcd_kit_set_bl!\n");
		return LCD_KIT_OK;
	}
	if (disp_info->bl_is_shield_backlight == true) {
		LCD_KIT_ERR("in finger down status, Not run lcd_kit_set_bl\n");
		return LCD_KIT_OK;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->quickly_sleep_out.support) {
		if (disp_info->quickly_sleep_out.panel_on_tag)
			lcd_kit_disp_on_check_delay();
	}
	first_screenon = lcd_kit_first_screenon(jank_last_bl_level, bl_level);
	if ((jank_last_bl_level == 0) && (bl_level != 0)) {
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_ON, "LCD_BACKLIGHT_ON,%u", bl_level);
		jank_last_bl_level = bl_level;
	} else if ((bl_level == 0) && (jank_last_bl_level != 0)) {
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_OFF, "LCD_BACKLIGHT_OFF");
		jank_last_bl_level = bl_level;
	}
	bl_flicker_detector_collect_upper_bl(bl_level);
	bl_flicker_detector_collect_algo_delta_bl(hisifd->de_info.blc_delta);
	bl_type = lcd_kit_get_bl_set_type(pinfo);
	switch (bl_type) {
	case BL_SET_BY_PWM:
		ret = hisi_pwm_set_backlight(hisifd, bl_level);
		break;
	case BL_SET_BY_BLPWM:
		ret = lcd_kit_blpwm_set_backlight(hisifd, bl_level);
		break;
	case BL_SET_BY_MIPI:
		if (hisifd->masklayer_flag == MASK_LAYER_SCREENON
			|| hisifd->masklayer_flag == CIRCLE_LAYER) {
			if (first_screenon) {
				LCD_KIT_INFO("first_screenon  open HBM\n");
				lcd_kit_set_hbm_for_screenon(pdev, 1);
				hisifd->hbm_is_opened = true;
			}
#ifdef LV_GET_LCDBK_ON
			mipi_level = g_mipi_level;
#endif
		} else {
			ret = lcd_kit_mipi_set_backlight(hisifd, bl_level);
		}
		break;
	default:
		LCD_KIT_ERR("not support bl_type\n");
		ret = -1;
		break;
	}
	LCD_KIT_INFO("bl_type = %d, bl_level = %d\n", bl_type, bl_level);
	if (first_screenon)
		de_ctrl_ic_dim(hisifd);
#ifdef LV_GET_LCDBK_ON
	save_light_to_sensorhub(mipi_level, bl_level);
#endif
	if (first_screenon)
		lcd_kit_poweric_detect(hisifd);
	return ret;
}

static int lcd_kit_esd_check(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_panel_ops *panel_ops = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("esd check pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	mutex_lock(&disp_info->mipi_lock);
	panel_ops = lcd_kit_panel_get_ops();
	if (panel_ops && panel_ops->lcd_esd_check) {
		ret = panel_ops->lcd_esd_check(hisifd);
		mutex_unlock(&disp_info->mipi_lock);
		return ret;
	}
	if (common_ops->esd_handle)
		ret = common_ops->esd_handle(hisifd);
	mutex_unlock(&disp_info->mipi_lock);
	return ret;
}

static void fastboot_check_dsm_ocuppy(void)
{
	u32 value = 0;
#if defined(CONFIG_HUAWEI_DSM)
	#define REC_LIMIT_TIMES (-1)
	int cur_rec_time = 0;
#endif

	lcd_kit_get_value_from_dts("huawei,lcd_panel_type",
		"fastboot_record_bit", &value);
	if (value & BIT(0)) {
		/* happen short in fastboot, notify dmd */
#if defined(CONFIG_HUAWEI_DSM)
		(void)lcd_dsm_client_record(lcd_dclient,
			"lp8556 happen short in fastboot\n",
			DSM_LCD_OVP_ERROR_NO,
			REC_LIMIT_TIMES, &cur_rec_time);
#endif
		LCD_KIT_INFO("lp8556 happen short\n");
	}
	if (value & BIT(1)) {
		/* logo checksum fail in fastboot, notify dmd */
#if defined(CONFIG_HUAWEI_DSM)
		(void)lcd_dsm_client_record(lcd_dclient,
			"checksum fail in fastboott\n",
			DSM_LCD_CHECKSUM_ERROR_NO,
			REC_LIMIT_TIMES, &cur_rec_time);
#endif
		LCD_KIT_INFO("logo checksum fail\n");
	}
	LCD_KIT_INFO("value = 0x%x", value);
}

static int lcd_kit_set_fastboot(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	// read fps gamma if needed
	lcd_kit_read_fps_gamma(hisifd);
	// backlight on
	hisi_lcd_backlight_on(pdev);
	// get blmaxnit
	if (common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC) {
		ret = lcd_kit_dsi_cmds_rx((void *)hisifd,
			(uint8_t *)&common_info->blmaxnit.lcd_kit_brightness_ddic_info,
			&common_info->blmaxnit.bl_maxnit_cmds);
		if (ret)
			LCD_KIT_ERR("read blmaxnit_reg error\n");
		LCD_KIT_INFO("lcd_kit_brightness_ddic_info = %d\n",
			common_info->blmaxnit.lcd_kit_brightness_ddic_info);
	}

	// lcd panel version
	if (disp_info->panel_version.support) {
		if (!lcd_kit_panel_version_init(hisifd))
			LCD_KIT_INFO("read panel version successful\n");
		else
			LCD_KIT_INFO("read panel version fail\n");
	}

	fastboot_check_dsm_ocuppy();

	/* update gamma */
	lcd_kit_update_gamma_from_tpic(hisifd);
	return LCD_KIT_OK;
}

static int lcd_kit_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	if (common_ops->dirty_region_handle) {
		mutex_lock(&disp_info->mipi_lock);
		ret = common_ops->dirty_region_handle(hisifd,
			(struct region_rect *)dirty);
		mutex_unlock(&disp_info->mipi_lock);
	}
	return ret;
}

static int lcd_kit_set_tcon_mode(struct platform_device *pdev, uint8_t mode)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (pdev == NULL) {
		LCD_KIT_ERR("pdev is null\n");
		return LCD_KIT_FAIL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	switch (mode) {
	case EN_DISPLAY_REGION_A:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_a_cmds);
		break;
	case EN_DISPLAY_REGION_B:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_b_cmds);
		break;
	case EN_DISPLAY_REGION_AB:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_ab_cmds);
		break;
	case EN_DISPLAY_REGION_AB_FOLDED:
		if (adapt_ops->mipi_tx)
			ret = adapt_ops->mipi_tx((void *)hisifd,
				&disp_info->cascade_ic.region_ab_fold_cmds);
		break;
	default:
		LCD_KIT_ERR("mode %d is not support\n", mode);
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	int ret = LCD_KIT_OK;

	if (disp_info->fps.support)
		ret = lcd_kit_updt_fps_scence(pdev, scence);
	return ret;
}

static int lcd_kit_fps_updt_handle(struct platform_device *pdev)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->fps.support)
		ret = lcd_kit_updt_fps(pdev);
	return ret;
}

static ssize_t lcd_kit_ce_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	int ret;
	unsigned long mode = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!pdev) {
		LCD_KIT_ERR("ce mode store pdev is null\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &mode);
	if (ret) {
		LCD_KIT_ERR("strict_strtoul error\n");
		return ret;
	}

	hisifd->user_scene_mode = (int)mode;
	if (common_ops->set_ce_mode)
		ret = common_ops->set_ce_mode(hisifd, mode);
	return count;
}

static ssize_t lcd_kit_rgbw_set_func(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;

	if (disp_info->rgbw.support)
		ret = lcd_kit_rgbw_set_handle(hisifd);
	return ret;
}

static int lcd_get_demura_func(struct platform_device *pdev,
	unsigned char dsi, unsigned char *out,
	unsigned char type, unsigned char len)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	if (!pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if ((!hisifd) || (!out)) {
		HISI_FB_ERR("invalid input param!\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->demura.support)
		ret = lcd_get_demura_handle(hisifd, dsi, out, type, len);
	return ret;
}

static int lcd_set_demura_func(struct platform_device *pdev,
	unsigned char type, const demura_set_info_t *info)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	if ((!pdev) || (!info)) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		HISI_FB_ERR("invalid input param!\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->demura.support)
		ret = lcd_set_demura_handle(hisifd, type, info);
	return ret;
}

static ssize_t lcd_kit_hbm_set_func(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;
	/* waiting fps max time */
	long timeout = msecs_to_jiffies(50);
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (common_info->dfr_info.fps_lock_command_support) {
		wait_event_interruptible_timeout(
			common_info->dfr_info.hbm_hist,
			common_info->dfr_info.fps_dfr_status,
			timeout);
		common_info->dfr_info.hbm_status = HBM_STATUS_DOING;
		down(&hisifd->blank_sem);
		if (!hisifd->panel_power_on) {
			up(&hisifd->blank_sem);
			HISI_FB_DEBUG("fb%d, power off!\n", hisifd->index);
			return ret;
		}
		hisifb_activate_vsync(hisifd);
	}
	if (common_ops->hbm_set_handle)
		ret = common_ops->hbm_set_handle(hisifd,
			hisifd->de_info.last_hbm_level,
			hisifd->de_info.hbm_dimming,
			hisifd->de_info.hbm_level,
			pinfo->fps);
	if (common_info->dfr_info.fps_lock_command_support) {
		common_info->dfr_info.hbm_status = HBM_STATUS_IDLE;
		wake_up_interruptible(&common_info->dfr_info.fps_hist);
		hisifb_deactivate_vsync(hisifd);
		up(&hisifd->blank_sem);
	}
	return ret;
}

static ssize_t lcd_kit_set_ic_dim_on(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	if (common_info->hbm.support) {
		if(hisifd->de_info.hbm_level != 0) {
			HISI_FB_INFO("now is hbm, hbm_level:%d,fp_hbm:%d\n",
				hisifd->de_info.hbm_level, common_info->hbm.hbm_if_fp_is_using);
			return ret;
		}
	}
	if (disp_info->bl_is_shield_backlight == true) {
		HISI_FB_INFO("fp is using\n");
		return ret;
	}
	if (common_ops->set_ic_dim_on)
		ret = common_ops->set_ic_dim_on(hisifd,
			pinfo->fps);
	return ret;
}



static ssize_t lcd_kit_cabc_store(struct platform_device *pdev, const char *buf,
	size_t count)
{
	ssize_t ret;
	struct hisi_fb_data_type *hisifd = NULL;
	unsigned long mode = 0;

	hisifd = platform_get_drvdata(pdev);
	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &mode);
	if (ret) {
		LCD_KIT_ERR("invalid data!\n");
		return ret;
	}
	if (common_ops->set_cabc_mode) {
		mutex_lock(&disp_info->mipi_lock);
		common_ops->set_cabc_mode(hisifd, mode);
		mutex_unlock(&disp_info->mipi_lock);
	}
	return count;
}

/*
 * Forced power-off will cause display error,
 * so need to send 10 command
 * before the long press power off first
 */
static void power_off_work(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_kit_adapt_ops *ops = NULL;
	int ret;

	(void *)work;
	if (!disp_info) {
		LCD_KIT_ERR("disp info null!\n");
		return;
	}
	if (disp_info->pwrkey_press.long_press_flag == false) {
		LCD_KIT_INFO("long press flag false!\n");
		return;
	}
	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("hisifd null!\n");
		return;
	}
	ops = lcd_kit_get_adapt_ops();
	if (!ops) {
		LCD_KIT_ERR("get adapt ops null!\n");
		return;
	}
	if (!hisifd->panel_power_on) {
		HISI_FB_INFO("fb%d is power off, stop send commands!\n", hisifd->index);
		return;
	}
	LCD_KIT_INFO("long press, power off lcd!\n");
	hisifb_vsync_disable_enter_idle(hisifd, true);
	hisifb_activate_vsync(hisifd);
	if (ops->mipi_tx)
		ret = ops->mipi_tx((void *)hisifd,
			&disp_info->pwrkey_press.cmds);
	hisifb_vsync_disable_enter_idle(hisifd, false);
	hisifb_deactivate_vsync(hisifd);
}

static unsigned int get_power_off_timer_val(void)
{
	unsigned int ret_time;
	unsigned int press_time;
	unsigned int val;
	unsigned int addr;

	if (!disp_info)
		return LONG_PRESS_10S_LCD_TIMER_LEN;
	addr = disp_info->pwrkey_press.rst_addr;
	press_time = hisi_pmic_reg_read(addr);
	LCD_KIT_INFO("press_time %u!\n", press_time);
	val = press_time & LONG_PRESS_RST_CONFIG_BIT;
	switch (val) {
	case LONG_PRESS_7S_RST_CONFIG:
		ret_time = LONG_PRESS_7S_LCD_TIMER_LEN;
		break;
	case LONG_PRESS_8S_RST_CONFIG:
		ret_time = LONG_PRESS_8S_LCD_TIMER_LEN;
		break;
	case LONG_PRESS_9S_RST_CONFIG:
		ret_time = LONG_PRESS_9S_LCD_TIMER_LEN;
		break;
	case LONG_PRESS_10S_RST_CONFIG:
		ret_time = LONG_PRESS_10S_LCD_TIMER_LEN;
		break;
	default:
		ret_time = LONG_PRESS_10S_LCD_TIMER_LEN;
		break;
	}
	return ret_time;
}

static int pwrkey_press_event_notifier(struct notifier_block *pwrkey_event_nb,
	unsigned long event, void *data)
{
	struct notifier_block *p = NULL;
	void *pd = NULL;
	unsigned int time;
	p = pwrkey_event_nb;
	pd = data;

	if (!disp_info)
		return LCD_KIT_OK;
	time = disp_info->pwrkey_press.timer_val;
	switch (event) {
	case HISI_PRESS_KEY_6S:
		disp_info->pwrkey_press.long_press_flag = true;
		schedule_delayed_work(&disp_info->pwrkey_press.pf_work,
			msecs_to_jiffies(time));
		break;
	case HISI_PRESS_KEY_UP:
		if (disp_info->pwrkey_press.long_press_flag == false)
			break;
		disp_info->pwrkey_press.long_press_flag = false;
		cancel_delayed_work_sync(&disp_info->pwrkey_press.pf_work);
		break;
	default:
		break;
	}
	return LCD_KIT_OK;
}

static void lcd_kit_register_power_key_notify(void)
{
	int ret;
	struct delayed_work *p_work = NULL;

	if (!disp_info)
		return;
	if (disp_info->pwrkey_press.support == false)
		return;

	p_work = &disp_info->pwrkey_press.pf_work;
	INIT_DELAYED_WORK(p_work, power_off_work);
	disp_info->pwrkey_press.timer_val = get_power_off_timer_val();
	disp_info->pwrkey_press.nb.notifier_call = pwrkey_press_event_notifier;
	ret = hisi_powerkey_register_notifier(&disp_info->pwrkey_press.nb);
	if (ret < 0)
		LCD_KIT_ERR("register power_key notifier failed!\n");
}

static ssize_t lcd_kit_color_param_get_func(struct hisi_fb_data_type *hisifd)
{
#define C_LMT_LEN 3
#define ROW_LEN 3
#define COLUMN_LEN 3
	struct hisi_panel_info *pinfo = NULL;
	struct lcd_kit_brightness_color_oeminfo *oeminfo = NULL;
	int i, j;

	if (!hisifd) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is NULL!\n");
		return LCD_KIT_FAIL;
	}
	oeminfo = lcd_kit_get_brightness_color_oeminfo();
	if (!disp_info->oeminfo.support ||
		!disp_info->oeminfo.brightness_color_uniform.support) {
		LCD_KIT_INFO("oeminfo not support\n");
		return LCD_KIT_OK;
	}
	hisifd->de_info.lcd_color_oeminfo.id_flag = oeminfo->id_flag;
	hisifd->de_info.lcd_color_oeminfo.tc_flag = oeminfo->tc_flag;

	hisifd->de_info.lcd_color_oeminfo.panel_id.modulesn =
		oeminfo->panel_id.modulesn;
	hisifd->de_info.lcd_color_oeminfo.panel_id.equipid =
		oeminfo->panel_id.equipid;
	hisifd->de_info.lcd_color_oeminfo.panel_id.modulemanufactdate =
		oeminfo->panel_id.modulemanufactdate;
	hisifd->de_info.lcd_color_oeminfo.panel_id.vendorid =
		oeminfo->panel_id.vendorid;
	for (i = 0; i < C_LMT_LEN; i++)
		hisifd->de_info.lcd_color_oeminfo.color_params.c_lmt[i] =
			oeminfo->color_params.c_lmt[i];
	for (i = 0; i < ROW_LEN; i++) {
		for (j = 0; j < COLUMN_LEN; j++) {
			hisifd->de_info.lcd_color_oeminfo.color_params.mxcc_matrix[i][j] =
				oeminfo->color_params.mxcc_matrix[i][j];
			hisifd->de_info.lcd_color_oeminfo.color_mdata.chroma_coordinates[i][j] =
				oeminfo->color_mdata.chroma_coordinates[i][j];
		}
	}
	hisifd->de_info.lcd_color_oeminfo.color_params.white_decay_luminace =
		oeminfo->color_params.white_decay_luminace;
	hisifd->de_info.lcd_color_oeminfo.color_mdata.white_luminance =
		oeminfo->color_mdata.white_luminance;
	return LCD_KIT_OK;
}

static void factory_init(struct hisi_panel_info *pinfo)
{
	if (runmode_is_factory()) {
		if (common_info->esd.fac_esd_support == 1) {
			common_info->esd.support = 1;
			pinfo->esd_enable = 1;
		} else {
			common_info->esd.support = 0;
			pinfo->esd_enable = 0;
		}
		pinfo->dirty_region_updt_support = 0;
		pinfo->prefix_ce_support = 0;
		pinfo->prefix_sharpness1D_support = 0;
		pinfo->prefix_sharpness2D_support = 0;
		pinfo->sbl_support = 0;
		pinfo->acm_support = 0;
		pinfo->acm_ce_support = 0;
		pinfo->comform_mode_support = 0;
		pinfo->color_temp_rectify_support = 0;
		pinfo->hiace_support = 0;
		pinfo->arsr1p_sharpness_support = 0;
		pinfo->blpwm_input_ena = 0;
		pinfo->gmp_support = 0;
		pinfo->vsync_ctrl_type = 0;
		pinfo->dither_support = 0;
		common_info->effect_on.support = 0;
		common_info->effect_color.mode &= BITS(31);
	}
}

/*
 * name:lcd_kit_probe
 * function:panel driver probe
 * @pdev:platform device
 */
static int lcd_kit_probe(struct platform_device *pdev)
{
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	int ret;
	struct lcd_kit_ops *lcd_ops = NULL;

	np = pdev->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node\n");
		return LCD_KIT_FAIL;
	}
#ifdef LCD_KIT_DEBUG_ENABLE
	if (dpd_init(pdev)) {
		LCD_KIT_INFO("sysfs is not ready!\n");
		return -EPROBE_DEFER;
	}
#endif
	LCD_KIT_INFO("enter probe!\n");
	pinfo = lcd_kit_data.panel_info;
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	/* adapt init */
	lcd_kit_adapt_init();
	bias_bl_ops_init();
	/* common init */
	if (common_ops->common_init)
		common_ops->common_init(np);
	/* utils init */
	lcd_kit_utils_init(np, pinfo);
	/* init factory mode */
	factory_init(pinfo);
#ifdef LCD_KIT_DEBUG_ENABLE
	if (is_dpd_mode())
		dpd_regu_init(pdev);
	else
		lcd_kit_power_init(pdev);
#else
	/* power init */
	lcd_kit_power_init(pdev);
#endif
	/* init panel ops */
	lcd_kit_panel_init();
	/* init debug */
#ifdef LCD_KIT_DEBUG_ENABLE
	lcd_kit_dbg_init();
#endif
	/* probe driver */
	if (hisi_fb_device_probe_defer(pinfo->type, pinfo->bl_set_type))
		goto err_probe_defer;
	pdev->id = 1;
	ret = platform_device_add_data(pdev, &lcd_kit_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		LCD_KIT_ERR("platform_device_add_data failed!\n");
		goto err_device_put;
	}
	hisi_fb_add_device(pdev);
#ifdef LCD_FACTORY_MODE
	lcd_factory_init(np);
#endif
	lcd_kit_register_power_key_notify();
	/* init fnode */
	lcd_kit_sysfs_init();
	/* get sn code */
	lcd_ops = lcd_kit_get_ops();
	if (lcd_ops) {
		if (lcd_ops->get_sn_code) {
			ret = lcd_ops->get_sn_code();
			if (ret < 0)
				LCD_KIT_ERR("get sn code failed!\n");
		}
	}
	LCD_KIT_INFO("exit probe!\n");
	return LCD_KIT_OK;

err_device_put:
	platform_device_put(pdev);
err_probe_defer:
	return -EPROBE_DEFER;
}

/*
 * probe match table
 */
static struct of_device_id lcd_kit_match_table[] = {
	{
		.compatible = "auo_otm1901a_5p2_1080p_video",
		.data = NULL,
	},
	{
	},
};

/*
 * panel platform driver
 */
static struct platform_driver lcd_kit_driver = {
	.probe = lcd_kit_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "lcd_kit_mipi_panel",
		.of_match_table = lcd_kit_match_table,
	},
};

static void transfer_power_config(uint32_t *in, int in_len,
	struct lcd_kit_array_data *out)
{
	uint32_t *buf = NULL;
	uint8_t i;

	if ((in == NULL) || (out == NULL)) {
		LCD_KIT_ERR("param invalid!\n");
		return;
	}
	buf = (uint32_t *)kzalloc(LCD_POWER_LEN * sizeof(uint32_t), GFP_KERNEL);
	if (!buf) {
		LCD_KIT_ERR("alloc buf fail\n");
		return;
	}
	for (i = 0; i < in_len; i++) {
		buf[i] = in[i];
		LCD_KIT_INFO("POWER : 0x%x\n", buf[i]);
	}
	out->buf = buf;
	out->cnt = LCD_POWER_LEN;
}

static void lcd_get_power_seq_from_dts(struct device_node *np)
{
	uint32_t power[LCD_POWER_LEN] = {0};

	if (np == NULL) {
		LCD_KIT_ERR("invalid input param!\n");
		return;
	}
	if (!of_property_read_u32_array(np, "lcd_vci", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_vci);
	if (!of_property_read_u32_array(np, "lcd_iovcc", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_iovcc);
	if (!of_property_read_u32_array(np, "lcd_vdd", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_vdd);
	if (!of_property_read_u32_array(np, "lcd_vsp", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_vsp);
	if (!of_property_read_u32_array(np, "lcd_vsn", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_vsn);
	if (!of_property_read_u32_array(np, "lcd_rst", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_rst);
	if (!of_property_read_u32_array(np, "lcd_te0", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_te0);
	if (!of_property_read_u32_array(np, "lcd_aod", power, LCD_POWER_LEN))
		transfer_power_config(power, LCD_POWER_LEN, &power_hdl->lcd_aod);
}

static void lcd_get_project_id(struct device_node *np)
{
	char *project_id = NULL;
	int len;

	project_id = (char *)of_get_property(np, "project_id", NULL);
	if (project_id) {
		len = strlen(project_id);
		len = (len > PROJECTID_LEN) ? PROJECTID_LEN : len;
		strncpy(disp_info->project_id.id, project_id, len);
	}
}

static int __init lcd_kit_init(void)
{
	int ret = LCD_KIT_OK;
	int len;
	struct device_node *np = NULL;

	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return ret;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_LCD_KIT_PANEL_TYPE);
		return LCD_KIT_FAIL;
	}
	lcd_get_project_id(np);
	lcd_get_power_seq_from_dts(np);
	if (of_property_read_u32(np, "board_version", &disp_info->board_version))
		disp_info->board_version = 0;
	LCD_KIT_INFO("disp_info->board_version = 0x%x\n", disp_info->board_version);
	lcd_kit_parse_u32(np, "product_id", &disp_info->product_id, 0);
	LCD_KIT_INFO("disp_info->product_id = %d", disp_info->product_id);
	disp_info->compatible = (char *)of_get_property(np, "lcd_panel_type", NULL);
	if (!disp_info->compatible) {
		LCD_KIT_ERR("can not get lcd kit compatible\n");
		return ret;
	}
	LCD_KIT_DEBUG("disp_info->compatible = %s\n", disp_info->compatible);
	len = strlen(disp_info->compatible);
	memset((char *)lcd_kit_driver.driver.of_match_table->compatible, 0,
		LCD_KIT_PANEL_COMP_LENGTH);
	strncpy((char *)lcd_kit_driver.driver.of_match_table->compatible,
		disp_info->compatible, len > (LCD_KIT_PANEL_COMP_LENGTH - 1) ?
		(LCD_KIT_PANEL_COMP_LENGTH - 1) : len);
	/* register driver */
	ret = platform_driver_register(&lcd_kit_driver);
	if (ret)
		LCD_KIT_ERR("platform_driver_register failed, error=%d!\n", ret);
	return ret;
}
module_init(lcd_kit_init);
