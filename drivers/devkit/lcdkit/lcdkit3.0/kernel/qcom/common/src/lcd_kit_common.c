/*
 * lcd_kit_common.c
 *
 * lcdkit common function for lcdkit
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "lcd_kit_parse.h"
#define LCD_ACTIVE_PANEL_BUTT 1
#define LCD_ACTIVE_PANEL 0
#define LCD_FOLDER_TYPE 1

#if defined CONFIG_HUAWEI_DSM
extern struct dsm_client *lcd_dclient;
#endif

#define BL_MAX 256
#define SKIP_SEND_EVENT 1

int lcd_kit_msg_level = MSG_LEVEL_INFO;
/* common info */
struct lcd_kit_common_info g_lcd_kit_common_info[LCD_ACTIVE_PANEL_BUTT];
/* common ops */
struct lcd_kit_common_ops g_lcd_kit_common_ops;
/* power handle */
struct lcd_kit_power_desc g_lcd_kit_power_handle[LCD_ACTIVE_PANEL_BUTT];
/* power on/off sequence */
static struct lcd_kit_power_seq g_lcd_kit_power_seq[LCD_ACTIVE_PANEL_BUTT];
/* esd error info */
struct lcd_kit_esd_error_info g_esd_error_info;
/* hw adapt ops */
static struct lcd_kit_adapt_ops *g_adapt_ops;
/* common lock */
struct lcd_kit_common_lock g_lcd_kit_common_lock;

/* dsi test */
#define RECORD_BUFLEN_DSI 200
#define REC_DMD_NO_LIMIT_DSI (-1)
char record_buf_dsi[RECORD_BUFLEN_DSI] = {'\0'};
int cur_rec_time_dsi = 0;

static int lcd_kit_get_proxmity_status(int data);

int lcd_kit_adapt_register(struct lcd_kit_adapt_ops *ops)
{
	if (g_adapt_ops) {
		LCD_KIT_ERR("g_adapt_ops has already been registered!\n");
		return LCD_KIT_FAIL;
	}
	g_adapt_ops = ops;
	LCD_KIT_INFO("g_adapt_ops register success!\n");
	return LCD_KIT_OK;
}

int lcd_kit_adapt_unregister(struct lcd_kit_adapt_ops *ops)
{
	if (g_adapt_ops == ops) {
		g_adapt_ops = NULL;
		LCD_KIT_INFO("g_adapt_ops unregister success!\n");
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("g_adapt_ops unregister fail!\n");
	return LCD_KIT_FAIL;
}

struct lcd_kit_adapt_ops *lcd_kit_get_adapt_ops(void)
{
	return g_adapt_ops;
}

struct lcd_kit_common_info *lcd_kit_get_common_info(void)
{
	return &g_lcd_kit_common_info[LCD_ACTIVE_PANEL];
}

struct lcd_kit_common_ops *lcd_kit_get_common_ops(void)
{
	return &g_lcd_kit_common_ops;
}

struct lcd_kit_power_desc *lcd_kit_get_power_handle(void)
{
	return &g_lcd_kit_power_handle[LCD_ACTIVE_PANEL];
}

struct lcd_kit_power_seq *lcd_kit_get_power_seq(void)
{
	return &g_lcd_kit_power_seq[LCD_ACTIVE_PANEL];
}

struct lcd_kit_common_lock *lcd_kit_get_common_lock(void)
{
	return &g_lcd_kit_common_lock;
}

static int lcd_kit_set_bias_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_bias_ops *bias_ops = NULL;

	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (enable) {
		if (power_hdl->lcd_vsp.buf == NULL) {
			LCD_KIT_ERR("can not get lcd voltage!\n");
			return LCD_KIT_FAIL;
		}
		if (bias_ops->set_bias_voltage)
			/* buf[2]:set voltage value */
			ret = bias_ops->set_bias_voltage(
				power_hdl->lcd_vsp.buf[POWER_VOL],
				power_hdl->lcd_vsn.buf[POWER_VOL]);
		if (ret)
			LCD_KIT_ERR("set_bias failed\n");
	} else {
		if (power_hdl->lcd_power_down_vsp.buf == NULL) {
			LCD_KIT_INFO("PowerDownVsp is not configured in xml!\n");
			return LCD_KIT_FAIL;
		}
		if (bias_ops->set_bias_power_down)
			/* buf[2]:set voltage value */
			ret = bias_ops->set_bias_power_down(
				power_hdl->lcd_power_down_vsp.buf[POWER_VOL],
				power_hdl->lcd_power_down_vsn.buf[POWER_VOL]);
		if (ret)
			LCD_KIT_ERR("power_down_set_bias failed!\n");
	}
	return ret;
}

static int lcd_kit_vci_power_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vci.buf) {
		LCD_KIT_ERR("can not get lcd vci!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_vci.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(LCD_KIT_VCI);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(LCD_KIT_VCI);
		}
		break;
	case REGULATOR_MODE:
		if (enable) {
			if (adapt_ops->regulator_enable)
				ret = adapt_ops->regulator_enable(LCD_KIT_VCI);
		} else {
			if (adapt_ops->regulator_disable)
				ret = adapt_ops->regulator_disable(LCD_KIT_VCI);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vci mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vci mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_iovcc_power_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_iovcc.buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_iovcc.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(LCD_KIT_IOVCC);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(LCD_KIT_IOVCC);
		}
		break;
	case REGULATOR_MODE:
		if (enable) {
			if (adapt_ops->regulator_enable)
				ret = adapt_ops->regulator_enable(LCD_KIT_IOVCC);
		} else {
			if (adapt_ops->regulator_disable)
				ret = adapt_ops->regulator_disable(LCD_KIT_IOVCC);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd iovcc mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd iovcc mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_vdd_power_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vdd.buf) {
		LCD_KIT_ERR("can not get lcd iovcc!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_vdd.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(LCD_KIT_VDD);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(LCD_KIT_VDD);
		}
		break;
	case REGULATOR_MODE:
		if (enable) {
			if (adapt_ops->regulator_enable)
				ret = adapt_ops->regulator_enable(LCD_KIT_VDD);
		} else {
			if (adapt_ops->regulator_disable)
				ret = adapt_ops->regulator_disable(LCD_KIT_VDD);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vdd mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vdd mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static void lcd_kit_set_ic_disable(int enable)
{
	int ret;
	struct lcd_kit_bias_ops *bias_ops = NULL;

	if (enable)
		return;
	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not bias_ops!\n");
		return;
	}

	if (!bias_ops->set_ic_disable) {
		LCD_KIT_ERR("set_ic_disable is null!\n");
		return;
	}

	ret = bias_ops->set_ic_disable();
	if (ret) {
		LCD_KIT_ERR("ic disbale fail !\n");
		return;
	}
	LCD_KIT_INFO("ic disbale successful\n");
}

static int lcd_kit_vsp_power_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vsp.buf) {
		LCD_KIT_ERR("can not get lcd vsp!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_vsp.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(LCD_KIT_VSP);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(LCD_KIT_VSP);
		}
		break;
	case REGULATOR_MODE:
		if (enable) {
			if (adapt_ops->regulator_enable)
				ret = adapt_ops->regulator_enable(LCD_KIT_VSP);
		} else {
			if (adapt_ops->regulator_disable)
				ret = adapt_ops->regulator_disable(LCD_KIT_VSP);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vsp mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vsp mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	lcd_kit_set_ic_disable(enable);
	return ret;
}

static int lcd_kit_vsn_power_ctrl(int enable)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_vsn.buf) {
		LCD_KIT_ERR("can not get lcd vsn!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_vsn.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				ret = adapt_ops->gpio_enable(LCD_KIT_VSN);
		} else {
			if (adapt_ops->gpio_disable)
				ret = adapt_ops->gpio_disable(LCD_KIT_VSN);
		}
		break;
	case REGULATOR_MODE:
		if (enable) {
			if (adapt_ops->regulator_enable)
				ret = adapt_ops->regulator_enable(LCD_KIT_VSN);
		} else {
			if (adapt_ops->regulator_disable)
				ret = adapt_ops->regulator_disable(LCD_KIT_VSN);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd vsn mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd vsn mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_aod_power_ctrl(int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_aod.buf) {
		LCD_KIT_ERR("can not get lcd lcd_aod!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_aod.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable)
				adapt_ops->gpio_enable(LCD_KIT_AOD);
		} else {
			if (adapt_ops->gpio_disable)
				adapt_ops->gpio_disable(LCD_KIT_AOD);
		}
		break;
	case REGULATOR_MODE:
		if (enable) {
			if (adapt_ops->regulator_enable)
				adapt_ops->regulator_enable(LCD_KIT_AOD);
		} else {
			if (adapt_ops->regulator_disable)
				adapt_ops->regulator_disable(LCD_KIT_AOD);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd lcd_aod mode is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd lcd_aod mode is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

int lcd_kit_set_bias_voltage(void)
{
	struct lcd_kit_bias_ops *bias_ops = NULL;
	int ret = LCD_KIT_OK;

	bias_ops = lcd_kit_get_bias_ops();
	if (!bias_ops) {
		LCD_KIT_ERR("can not get bias_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* set bias voltage */
	if (bias_ops->set_bias_voltage)
		ret = bias_ops->set_bias_voltage(power_hdl->lcd_vsp.buf[POWER_VOL],
			power_hdl->lcd_vsn.buf[POWER_VOL]);
	return ret;
}

static void lcd_kit_proximity_record_time(void)
{
	struct timeval *reset_tv = NULL;

	if (common_info == NULL)
		return;
	if (lcd_kit_get_proxmity_status(LCD_RESET_HIGH) != TP_PROXMITY_ENABLE ||
		common_info->thp_proximity.after_reset_delay_min == 0)
		return;
	reset_tv = &(common_info->thp_proximity.lcd_reset_record_tv);
	do_gettimeofday(reset_tv);
	LCD_KIT_INFO("record lcd reset power on time");
}

int lcd_kit_set_mipi_switch_ctrl(int enable)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not get adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_mipi_switch.buf == NULL) {
		LCD_KIT_ERR("can not get lcd mipi switch!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_mipi_switch.buf[0]) {
	case GPIO_MODE:
		if (enable) {
			if (adapt_ops->gpio_enable_nolock)
				adapt_ops->gpio_enable_nolock(LCD_KIT_MIPI_SWITCH);
		} else {
			if (adapt_ops->gpio_disable_nolock)
				adapt_ops->gpio_disable_nolock(LCD_KIT_MIPI_SWITCH);
		}
		break;
	case NONE_MODE:
		LCD_KIT_DEBUG("lcd mipi switch is none mode\n");
		break;
	default:
		LCD_KIT_ERR("lcd mipi switch is not normal\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int lcd_kit_reset_power_on(void)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_rst.buf) {
		LCD_KIT_ERR("can not get lcd reset!\n");
		return LCD_KIT_FAIL;
	}

	switch (power_hdl->lcd_rst.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_enable)
			ret = adapt_ops->gpio_enable(LCD_KIT_RST);
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n", power_hdl->lcd_rst.buf[POWER_MODE]);
		break;
	}
	lcd_kit_proximity_record_time();
	return ret;
}

static int lcd_kit_reset_power_off(void)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!power_hdl->lcd_rst.buf) {
		LCD_KIT_ERR("can not get lcd reset!\n");
		return LCD_KIT_FAIL;
	}
	switch (power_hdl->lcd_rst.buf[POWER_MODE]) {
	case GPIO_MODE:
		if (adapt_ops->gpio_disable)
			ret = adapt_ops->gpio_disable(LCD_KIT_RST);
		break;
	default:
		LCD_KIT_ERR("not support type:%d\n",
			power_hdl->lcd_rst.buf[POWER_MODE]);
		break;
	}
	return ret;
}

int lcd_kit_reset_power_ctrl(int enable)
{
	if (enable == LCD_RESET_HIGH)
		return lcd_kit_reset_power_on();
	else
		return lcd_kit_reset_power_off();
}

static int lcd_kit_on_cmds(void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_off_cmds(void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_check_reg_report_dsm(void *hld,
	struct lcd_kit_check_reg_dsm *check_reg_dsm)
{
	int ret = LCD_KIT_OK;
	uint8_t read_value[MAX_REG_READ_COUNT] = {0};
	int i;
	char *expect_ptr = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!hld || !check_reg_dsm) {
		LCD_KIT_ERR("null pointer!\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!check_reg_dsm->support) {
		LCD_KIT_DEBUG("not support check reg dsm\n");
		return ret;
	}
	expect_ptr = (char *)check_reg_dsm->value.buf;
	if (adapt_ops->mipi_rx)
		ret = adapt_ops->mipi_rx(hld, read_value, MAX_REG_READ_COUNT - 1,
			&check_reg_dsm->cmds);
	if (ret == LCD_KIT_OK) {
		for (i = 0; i < check_reg_dsm->cmds.cmd_cnt; i++) {
			if (!check_reg_dsm->support_dsm_report) {
				LCD_KIT_INFO("read_value[%d] = 0x%x!\n",
					i, read_value[i]);
				continue;
			}
			if ((char)read_value[i] != expect_ptr[i]) {
				ret = LCD_KIT_FAIL;
				LCD_KIT_ERR("read_value[%d] = 0x%x, but expect_ptr[%d] = 0x%x!\n",
					i, read_value[i], i, expect_ptr[i]);
#if defined CONFIG_HUAWEI_DSM
				dsm_client_record(lcd_dclient, "lcd register read_value[%d] = 0x%x, but expect_ptr[%d] = 0x%x!\n",
					i, read_value[i], i, expect_ptr[i]);
#endif
				break;
			}
			LCD_KIT_INFO("read_value[%d] = 0x%x same with expect value!\n",
				i, read_value[i]);
		}
	} else {
		LCD_KIT_ERR("mipi read error!\n");
	}
	if (ret != LCD_KIT_OK) {
		if (check_reg_dsm->support_dsm_report) {
#if defined CONFIG_HUAWEI_DSM
			if (dsm_client_ocuppy(lcd_dclient))
				return ret;
			dsm_client_notify(lcd_dclient, DSM_LCD_STATUS_ERROR_NO);
#endif
		}
	}
	return ret;
}

void lcd_kit_proxmity_proc(int enable)
{
	long delta_time;
	int delay_margin;
	struct timeval tv;
	struct timeval *reset_tv = NULL;

	if (common_info == NULL)
		return;
	if (lcd_kit_get_proxmity_status(enable) != TP_PROXMITY_ENABLE ||
		common_info->thp_proximity.after_reset_delay_min == 0)
		return;
	memset(&tv, 0, sizeof(struct timeval));
	do_gettimeofday(&tv);
	reset_tv = &(common_info->thp_proximity.lcd_reset_record_tv);
	/* change s to us */
	delta_time = (tv.tv_sec - reset_tv->tv_sec) * 1000000 +
		tv.tv_usec - reset_tv->tv_usec;
	/* change us to ms */
	delta_time /= 1000;
	if (delta_time >= common_info->thp_proximity.after_reset_delay_min)
		return;
	delay_margin = common_info->thp_proximity.after_reset_delay_min - delta_time;
	if (delay_margin > common_info->thp_proximity.after_reset_delay_min ||
		delay_margin > MAX_MARGIN_DELAY)
		return;
	lcd_kit_delay(delay_margin, LCD_KIT_WAIT_MS, true);
	LCD_KIT_INFO("delay_margin:%d ms\n", delay_margin);
}

static int lcd_kit_mipi_power_ctrl(void *hld, int enable)
{
	int ret = LCD_KIT_OK;
	int ret_check_reg;

	if (enable) {
		ret = lcd_kit_on_cmds(hld);
		ret_check_reg = lcd_kit_check_reg_report_dsm(hld,
			&common_info->check_reg_on);
		if (ret_check_reg != LCD_KIT_OK)
			LCD_KIT_ERR("power on check reg error!\n");
	} else {
		ret = lcd_kit_off_cmds(hld);
		ret_check_reg = lcd_kit_check_reg_report_dsm(hld,
			&common_info->check_reg_off);
		if (ret_check_reg != LCD_KIT_OK)
			LCD_KIT_ERR("power off check reg error!\n");
	}
	return ret;
}

static int lcd_kit_aod_enter_ap_cmds(void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	/* aod enter ap code */
	if (adapt_ops->mipi_tx) {
		ret = adapt_ops->mipi_tx(hld, &common_info->aod_exit_dis_on_cmds);
		if (ret)
			LCD_KIT_ERR("send aod exit disp on cmds error\n");
	}
	return ret;
}

static int lcd_kit_aod_mipi_ctrl(void *hld, int enable)
{
	int ret = LCD_KIT_OK;
	if (enable)
		ret = lcd_kit_aod_enter_ap_cmds(hld);
	return ret;
}

static int lcd_kit_ts_resume(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_RESUME_DEVICE, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_RESUME_DEVICE, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_after_resume(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_AFTER_RESUME, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_AFTER_RESUME, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_suspend(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync) {
		ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, SHORT_SYNC);
		ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, SHORT_SYNC);
	} else {
		ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, NO_SYNC);
		ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, NO_SYNC);
	}
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_early_suspend(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_EARLY_SUSPEND, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_EARLY_SUSPEND, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_ts_2nd_power_off(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_power_notify(TS_2ND_POWER_OFF, SHORT_SYNC);
	else
		ret = ts_ops->ts_power_notify(TS_2ND_POWER_OFF, NO_SYNC);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}

static int lcd_kit_early_ts_event(int enable, int sync)
{
	if (enable)
		return lcd_kit_ts_resume(sync);
	return lcd_kit_ts_early_suspend(sync);
}

static int lcd_kit_later_ts_event(int enable, int sync)
{
	if (enable)
		return lcd_kit_ts_after_resume(sync);
	return lcd_kit_ts_suspend(sync);
}

static int lcd_kit_2nd_power_off_ts_event(int enable, int sync)
{
	return lcd_kit_ts_2nd_power_off(sync);
}

int lcd_kit_get_pt_mode(void)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int status = 0;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	if (lcd_ops->get_pt_station_status)
		status = lcd_ops->get_pt_station_status();
	LCD_KIT_INFO("[pt_mode] get status %d\n", status);
	return status;
}

bool lcd_kit_get_thp_afe_status(struct timeval *record_tv)
{
	struct ts_kit_ops *ts_ops = NULL;
	bool thp_afe_status = true;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return thp_afe_status;
	}
	if (ts_ops->get_afe_status) {
		thp_afe_status = ts_ops->get_afe_status(record_tv);
		LCD_KIT_INFO("get afe status %d\n", thp_afe_status);
	}
	return thp_afe_status;
}

static int lcd_kit_get_proxmity_status(int data)
{
	struct ts_kit_ops *ts_ops = NULL;
	static bool ts_get_proxmity_flag = true;

	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("[Proximity_feature] not support\n");
		return TP_PROXMITY_DISABLE;
	}
	if (data) {
		ts_get_proxmity_flag = true;
		LCD_KIT_INFO("[Proximity_feature] get status %d\n",
			common_info->thp_proximity.work_status );
		return common_info->thp_proximity.work_status ;
	}
	if (ts_get_proxmity_flag == false) {
		LCD_KIT_INFO("[Proximity_feature] get status %d\n",
			common_info->thp_proximity.work_status );
		return common_info->thp_proximity.work_status ;
	}
	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return TP_PROXMITY_DISABLE;
	}
	if (ts_ops->get_tp_proxmity) {
		common_info->thp_proximity.work_status = (int)ts_ops->get_tp_proxmity();
		LCD_KIT_INFO("[Proximity_feature] get status %d\n",
				common_info->thp_proximity.work_status );
	}
	ts_get_proxmity_flag = false;
	return common_info->thp_proximity.work_status;
}

static int lcd_kit_gesture_mode(void)
{
	struct ts_kit_ops *ts_ops = NULL;
	int status = 0;
	int ret;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return 0;
	}
	if (ts_ops->get_tp_status_by_type) {
		ret = ts_ops->get_tp_status_by_type(TS_GESTURE_FUNCTION, &status);
		if (ret) {
			LCD_KIT_INFO("get gesture function fail\n");
			return 0;
		}
	}
	LCD_KIT_INFO("[gesture_mode] get status %d\n", status);
	return status;
}

static int lcd_kit_panel_is_power_on(void)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int mode = 0;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("ts_ops is null\n");
		return 0;
	}
	if (lcd_ops->get_panel_power_status)
		mode = lcd_ops->get_panel_power_status();
	return mode;
}

static bool lcd_kit_event_skip_delay(void *hld,
	uint32_t event, uint32_t data)
{
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return false;
	}
	if (lcd_ops->panel_event_skip_delay)
		return lcd_ops->panel_event_skip_delay(hld, event, data);
	return false;
}

static int lcd_kit_avdd_mipi_ctrl(void *hld, int enable)
{
	struct lcd_kit_ops *lcd_ops = NULL;
	int ret = LCD_KIT_OK;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	if (lcd_ops->avdd_mipi_ctrl)
		ret = lcd_ops->avdd_mipi_ctrl(hld, enable);
	return ret;
}

static bool lcd_kit_event_skip_send(void *hld,
	uint32_t event, uint32_t enable)
{
	return false;
}

static int lcd_kit_gesture_event_handler(uint32_t event, uint32_t data, uint32_t delay)
{
	int ret = LCD_KIT_OK;
	int type = 0;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("gesture event = %d, data = %d, delay = %d\n", event, data, delay);
	switch (event) {
	case EVENT_VSP:
		type = LCD_KIT_VSP;
		break;
	case EVENT_VSN:
		type = LCD_KIT_VSN;
		break;
	case EVENT_RESET:
		type = LCD_KIT_RST;
		break;
	case EVENT_EARLY_TS:
		type = LCD_KIT_TP_RST;
		break;
	default:
		return LCD_KIT_FAIL;
	}

	if (data == 0)
		adapt_ops->gpio_disable(type);
	else
		adapt_ops->gpio_enable(type);

	lcd_kit_delay(delay, LCD_KIT_WAIT_MS, true);
	return ret;
}

static int lcd_kit_ft_gesture_mode_power_on(void)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct lcd_kit_array_data *gesture_pevent = NULL;

	if (common_info->tp_gesture_sequence_flag && lcd_kit_gesture_mode()) {
		/* FT8201 gesture mode power on timing */
		gesture_pevent = power_seq->gesture_power_on_seq.arry_data;
		for (i = 0; i < power_seq->gesture_power_on_seq.cnt; i++) {
			if (!gesture_pevent || !gesture_pevent->buf) {
				LCD_KIT_ERR("gesture_pevent is null!\n");
				return LCD_KIT_FAIL;
			}
			ret = lcd_kit_gesture_event_handler(gesture_pevent->buf[EVENT_NUM],
				gesture_pevent->buf[EVENT_DATA], gesture_pevent->buf[EVENT_DELAY]);
			if (ret) {
				LCD_KIT_ERR("send gesture_pevent 0x%x not exist!\n",
					gesture_pevent->buf[EVENT_NUM]);
				break;
			}
			gesture_pevent++;
		}
		return true;
	}
	return ret;
}

static int lcd_kit_event_should_send(void *hld, uint32_t event, uint32_t data)
{
	int ret = 0;

	if ((event == EVENT_IOVCC) && (data != 0)) {
		ret = lcd_kit_ft_gesture_mode_power_on();
		if (ret) {
			LCD_KIT_INFO("It is in gesture mode\n");
			return ret;
		}
	}

	if (lcd_kit_event_skip_send(hld, event, data))
		return SKIP_SEND_EVENT;

	switch (event) {
	case EVENT_VCI:
	case EVENT_IOVCC:
	case EVENT_VSP:
	case EVENT_VSN:
	case EVENT_VDD:
	case EVENT_BIAS:
		return (lcd_kit_get_pt_mode() ||
			lcd_kit_get_proxmity_status(data) ||
			((uint32_t)lcd_kit_gesture_mode() &&
			(common_info->ul_does_lcd_poweron_tp)));
	case EVENT_RESET:
		if (data && common_info->panel_on_always_need_reset) {
			return ret;
		} else {
			return (lcd_kit_get_pt_mode() ||
				lcd_kit_get_proxmity_status(data) ||
				((uint32_t)lcd_kit_gesture_mode() &&
				(common_info->ul_does_lcd_poweron_tp)));
		}
	case EVENT_MIPI:
		if (data)
			return lcd_kit_panel_is_power_on();
	break;
	case EVENT_AOD_MIPI:
		if (data)
			return !lcd_kit_panel_is_power_on();
		break;
	default:
		ret = 0;
	break;
	}
	return ret;
}

int lcd_kit_event_handler(void *hld, uint32_t event, uint32_t data,
	uint32_t delay)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("event = %d, data = %d, delay = %d\n", event, data, delay);
	if (lcd_kit_event_should_send(hld, event, data)) {
		LCD_KIT_INFO("It is in pt mode or gesture mode\n");
		return ret;
	}
	switch (event) {
	case EVENT_VCI:
		ret = lcd_kit_vci_power_ctrl(data);
		break;
	case EVENT_IOVCC:
		ret = lcd_kit_iovcc_power_ctrl(data);
		break;
	case EVENT_VSP:
		ret = lcd_kit_vsp_power_ctrl(data);
		break;
	case EVENT_VSN:
		ret = lcd_kit_vsn_power_ctrl(data);
		break;
	case EVENT_RESET:
		ret = lcd_kit_reset_power_ctrl(data);
		break;
	case EVENT_MIPI:
		ret = lcd_kit_mipi_power_ctrl(hld, data);
		break;
	case EVENT_EARLY_TS:
		lcd_kit_early_ts_event(data, delay);
		break;
	case EVENT_LATER_TS:
		lcd_kit_later_ts_event(data, delay);
		break;
	case EVENT_VDD:
		ret = lcd_kit_vdd_power_ctrl(data);
		break;
	case EVENT_AOD:
		ret = lcd_kit_aod_power_ctrl(data);
		break;
	case EVENT_NONE:
		LCD_KIT_INFO("none event\n");
		break;
	case EVENT_BIAS:
		lcd_kit_set_bias_ctrl(data);
		break;
	case EVENT_AOD_MIPI:
		lcd_kit_aod_mipi_ctrl(hld, data);
		break;
	case EVENT_MIPI_SWITCH:
		ret = lcd_kit_set_mipi_switch_ctrl(data);
		break;
	case EVENT_AVDD_MIPI:
		lcd_kit_avdd_mipi_ctrl(hld, data);
		break;
	case EVENT_2ND_POWER_OFF_TS:
		lcd_kit_2nd_power_off_ts_event(data, delay);
		break;
	default:
		LCD_KIT_INFO("event not exist\n");
		break;
	}
	/* In the case of aod exit, no delay is required */
	if (!lcd_kit_event_skip_delay(hld, event, data))
		lcd_kit_delay(delay, LCD_KIT_WAIT_MS, true);
	return ret;
}

static int lcd_kit_panel_power_on(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->power_on_seq.arry_data;
	for (i = 0; i < power_seq->power_on_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_panel_on_lp(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_on_lp_seq.arry_data;
	for (i = 0; i < power_seq->panel_on_lp_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_panel_on_hs(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_on_hs_seq.arry_data;
	for (i = 0; i < power_seq->panel_on_hs_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

int lcd_kit_panel_off_hs(void *hld)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_off_hs_seq.arry_data;
	for (i = 0; i < power_seq->panel_off_hs_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

int lcd_kit_panel_off_lp(void *hld)
{
	int ret = LCD_KIT_OK;
	int i = 0;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->panel_off_lp_seq.arry_data;
	for (i = 0; i < power_seq->panel_off_lp_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

int lcd_kit_panel_power_off(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->power_off_seq.arry_data;
	for (i = 0; i < power_seq->power_off_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	if (common_info->set_vss.support) {
		common_info->set_vss.power_off = 1;
		common_info->set_vss.new_backlight = 0;
	}
	return ret;
}

int lcd_kit_panel_only_power_off(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	struct lcd_kit_array_data *pevent = NULL;

	pevent = power_seq->only_power_off_seq.arry_data;
	for (i = 0; i < power_seq->only_power_off_seq.cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			return LCD_KIT_FAIL;
		}
		ret = lcd_kit_event_handler(hld, pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA], pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			break;
		}
		pevent++;
	}
	return ret;
}

static int lcd_kit_hbm_enable(void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enable hbm and open dimming */
	if (common_info->dfr_info.fps_lock_command_support &&
		(fps_status == LCD_KIT_FPS_HIGH)) {
		ret = adapt_ops->mipi_tx(hld,
			&common_info->dfr_info.cmds[FPS_90_HBM_DIM]);
	} else if (common_info->hbm.enter_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.enter_cmds);
	}
	return ret;
}

static int lcd_kit_enter_fp_hbm_extern(void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enable fp hbm*/
	if (common_info->hbm.fp_enter_extern_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.fp_enter_extern_cmds);
	return ret;
}

static int lcd_kit_exit_fp_hbm_extern(void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	if (adapt_ops == NULL || adapt_ops->mipi_tx == NULL) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* recover when exit fp hbm*/
	if (common_info->hbm.fp_exit_extern_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.fp_exit_extern_cmds);
	return ret;
}

static int lcd_kit_hbm_disable(void *hld)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* exit hbm */
	if (common_info->hbm.exit_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.exit_cmds);
	return ret;
}

static int lcd_kit_hbm_set_level(void *hld, int level)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* prepare */
	if (common_info->hbm.hbm_prepare_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(hld,
			&common_info->hbm.hbm_prepare_cmds);
	/* set hbm level */
	if (common_info->hbm.hbm_cmds.cmds != NULL) {
		if (common_info->hbm.hbm_special_bit_ctrl ==
			LCD_KIT_HIGH_12BIT_CTL_HBM_SUPPORT) {
			/* Set high 12bit hbm level, low 4bit set zero */
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
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.hbm_cmds);
	}
	/* post */
	if (common_info->hbm.hbm_post_cmds.cmds != NULL)
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.hbm_post_cmds);
	return ret;
}

static int lcd_kit_hbm_dim_disable(void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if ((fps_status == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(hld,
			&common_info->dfr_info.cmds[FPS_90_NORMAL_NO_DIM]);
	} else if (common_info->hbm.exit_dim_cmds.cmds != NULL) {
		/* close dimming */
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.exit_dim_cmds);
	}
	return ret;
}

static int lcd_kit_hbm_dim_enable(void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* open dimming exit hbm*/
	if ((fps_status == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(hld,
			&common_info->dfr_info.cmds[FPS_90_NORMAL_DIM]);
	} else if (common_info->hbm.enter_dim_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.enter_dim_cmds);
	}
	return ret;
}

static int lcd_kit_hbm_enable_no_dimming(void *hld, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	/* enable hbm and close dimming */
	if ((fps_status == LCD_KIT_FPS_HIGH) &&
		common_info->dfr_info.fps_lock_command_support) {
		ret = adapt_ops->mipi_tx(hld,
			&common_info->dfr_info.cmds[FPS_90_HBM_NO_DIM]);
	} else if (common_info->hbm.enter_no_dim_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(hld,
			&common_info->hbm.enter_no_dim_cmds);
	}
	return ret;
}

static void lcd_kit_hbm_print_count(int last_hbm_level, int hbm_level)
{
	static int count;
	int level_delta = 60;

	if (abs(hbm_level - last_hbm_level) > level_delta) {
		if (count == 0)
			LCD_KIT_INFO("last hbm_level=%d!\n", last_hbm_level);
		count = 5;
	}
	if (count > 0) {
		count--;
		LCD_KIT_INFO("hbm_level=%d!\n", hbm_level);
	} else {
		LCD_KIT_DEBUG("hbm_level=%d!\n", hbm_level);
	}
}

static int check_if_fp_using_hbm(void)
{
	int ret = LCD_KIT_OK;

	if (common_info->hbm.hbm_fp_support) {
		if (common_info->hbm.hbm_if_fp_is_using)
			ret = LCD_KIT_FAIL;
	}

	return ret;
}

static int lcd_kit_hbm_set_handle(void *hld, int last_hbm_level,
	int hbm_dimming, int hbm_level, int fps_status)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if ((!hld) || (hbm_level < 0) || (hbm_level > HBM_SET_MAX_LEVEL)) {
		LCD_KIT_ERR("input param invalid, hbm_level %d!\n", hbm_level);
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.support) {
		LCD_KIT_DEBUG("not support hbm\n");
		return ret;
	}
	mutex_lock(&COMMON_LOCK->hbm_lock);
	common_info->hbm.hbm_level_current = hbm_level;
	if (check_if_fp_using_hbm() < 0) {
		LCD_KIT_INFO("fp is using, exit!\n");
		mutex_unlock(&COMMON_LOCK->hbm_lock);
		return ret;
	}
	if (hbm_level > 0) {
		if (last_hbm_level == 0) {
			/* enable hbm */
			lcd_kit_hbm_enable(hld, fps_status);
			if (!hbm_dimming)
				lcd_kit_hbm_enable_no_dimming(hld, fps_status);
		} else {
			lcd_kit_hbm_print_count(last_hbm_level, hbm_level);
		}
		 /* set hbm level */
		lcd_kit_hbm_set_level(hld, hbm_level);
	} else {
		if (last_hbm_level == 0) {
			/* disable dimming */
			lcd_kit_hbm_dim_disable(hld, fps_status);
		} else {
			/* exit hbm */
			if (hbm_dimming)
				lcd_kit_hbm_dim_enable(hld, fps_status);
			else
				lcd_kit_hbm_dim_disable(hld, fps_status);
			lcd_kit_hbm_disable(hld);
		}
	}
	mutex_unlock(&COMMON_LOCK->hbm_lock);
	return ret;
}

static int lcd_kit_get_panel_name(char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", common_info->panel_name);
}

static u32 lcd_kit_get_blmaxnit(void)
{
	u32 bl_max_nit = 0;
	u32 lcd_kit_brightness_ddic_info;

	lcd_kit_brightness_ddic_info =
		common_info->blmaxnit.lcd_kit_brightness_ddic_info;
	if (common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC &&
		lcd_kit_brightness_ddic_info > BL_MIN &&
		lcd_kit_brightness_ddic_info < BL_MAX) {
		if (lcd_kit_brightness_ddic_info < BL_REG_NOUSE_VALUE)
			bl_max_nit = lcd_kit_brightness_ddic_info +
				common_info->bl_max_nit_min_value;
		else
			bl_max_nit = lcd_kit_brightness_ddic_info +
				common_info->bl_max_nit_min_value - 1;
	} else {
		bl_max_nit = common_info->actual_bl_max_nit;
	}
	return bl_max_nit;
}

static int lcd_kit_get_panel_info(char *buf)
{
#define PANEL_MAX 10
	int ret;
	char panel_type[PANEL_MAX] = {0};
	struct lcd_kit_bl_ops *bl_ops = NULL;
	char *bl_type = " ";

	if (common_info->panel_type == LCD_TYPE)
		strncpy(panel_type, "LCD", strlen("LCD"));
	else if (common_info->panel_type == AMOLED_TYPE)
		strncpy(panel_type, "AMOLED", strlen("AMOLED"));
	else
		strncpy(panel_type, "INVALID", strlen("INVALID"));
	common_info->actual_bl_max_nit = lcd_kit_get_blmaxnit();
	bl_ops = lcd_kit_get_bl_ops();
	if ((bl_ops != NULL) && (bl_ops->name != NULL))
		bl_type = bl_ops->name;
	ret = snprintf(buf, PAGE_SIZE,
		"blmax:%u,blmin:%u,blmax_nit_actual:%d,blmax_nit_standard:%d,lcdtype:%s,bl_type:%s\n",
		common_info->bl_level_max, common_info->bl_level_min,
		common_info->actual_bl_max_nit, common_info->bl_max_nit,
		panel_type, bl_type);
	return ret;
}

static int lcd_kit_get_cabc_mode(char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->cabc.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", common_info->cabc.mode);
	return ret;
}

static int lcd_kit_set_cabc_mode(void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->cabc.support) {
		LCD_KIT_DEBUG("not support cabc\n");
		return ret;
	}
	switch (mode) {
	case CABC_OFF_MODE:
		ret = adapt_ops->mipi_tx(hld, &common_info->cabc.cabc_off_cmds);
		break;
	case CABC_UI:
		ret = adapt_ops->mipi_tx(hld, &common_info->cabc.cabc_ui_cmds);
		break;
	case CABC_STILL:
		ret = adapt_ops->mipi_tx(hld, &common_info->cabc.cabc_still_cmds);
		break;
	case CABC_MOVING:
		ret = adapt_ops->mipi_tx(hld, &common_info->cabc.cabc_moving_cmds);
		break;
	default:
		return LCD_KIT_FAIL;
	}
	common_info->cabc.mode = mode;
	LCD_KIT_INFO("cabc.support = %d,cabc.mode = %d\n",
		common_info->cabc.support, common_info->cabc.mode);
	return ret;
}

static void lcd_kit_record_esd_error_info(int read_reg_index, int read_reg_val,
	int expect_reg_val)
{
	int reg_index = g_esd_error_info.esd_error_reg_num;

	if ((reg_index + 1) <= MAX_REG_READ_COUNT) {
		g_esd_error_info.esd_reg_index[reg_index] = read_reg_index;
		g_esd_error_info.esd_error_reg_val[reg_index] = read_reg_val;
		g_esd_error_info.esd_expect_reg_val[reg_index] = expect_reg_val;
		g_esd_error_info.esd_error_reg_num++;
	}
}

static int lcd_kit_judge_esd(unsigned char type, unsigned char read_val,
	unsigned char expect_val)
{
	int ret = 0;

	switch (type) {
	case ESD_UNEQUAL:
		if (read_val != expect_val)
			ret = 1;
		break;
	case ESD_EQUAL:
		if (read_val == expect_val)
			ret = 1;
		break;
	case ESD_BIT_VALID:
		if (read_val & expect_val)
			ret = 1;
		break;
	default:
		if (read_val != expect_val)
			ret = 1;
		break;
	}
	return ret;
}

static int lcd_kit_esd_handle(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	char read_value[MAX_REG_READ_COUNT] = {0};
	char expect_value, judge_type;
	u32 *esd_value = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->esd.support) {
		LCD_KIT_DEBUG("not support esd\n");
		return ret;
	}
	if (common_info->esd.status == ESD_STOP) {
		LCD_KIT_ERR("bypass esd check\n");
		return LCD_KIT_OK;
	}
	esd_value = common_info->esd.value.buf;
	ret = adapt_ops->mipi_rx(hld, read_value, MAX_REG_READ_COUNT - 1,
		&common_info->esd.cmds);
	if (ret) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return ret;
	}
	for (i = 0; i < common_info->esd.value.cnt; i++) {
		judge_type = (esd_value[i] >> 8) & 0xFF;
		expect_value = esd_value[i] & 0xFF;
		if (lcd_kit_judge_esd(judge_type, read_value[i], expect_value)) {
			lcd_kit_record_esd_error_info(i, (int)read_value[i],
				expect_value);
			LCD_KIT_ERR("read_value[%d] = 0x%x, but expect_value = 0x%x!\n",
				i, read_value[i], expect_value);
			ret = 1;
			break;
		}
		LCD_KIT_INFO("judge_type = %d, esd_value[%d] = 0x%x, read_value[%d] = 0x%x, expect_value = 0x%x\n",
			judge_type, i, esd_value[i], i, read_value[i], expect_value);
	}
	LCD_KIT_INFO("esd check result:%d\n", ret);
	return ret;
}

static int lcd_kit_get_ce_mode(char *buf)
{
	int ret = LCD_KIT_OK;

	if (buf == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->ce.support)
		ret = snprintf(buf, PAGE_SIZE,  "%d\n", common_info->ce.mode);
	return ret;
}

static int lcd_kit_set_ce_mode(void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->ce.support) {
		LCD_KIT_DEBUG("not support ce\n");
		return ret;
	}
	switch (mode) {
	case CE_OFF_MODE:
		ret = adapt_ops->mipi_tx(hld, &common_info->ce.off_cmds);
		break;
	case CE_SRGB:
		ret = adapt_ops->mipi_tx(hld, &common_info->ce.srgb_cmds);
		break;
	case CE_USER:
		ret = adapt_ops->mipi_tx(hld, &common_info->ce.user_cmds);
		break;
	case CE_VIVID:
		ret = adapt_ops->mipi_tx(hld, &common_info->ce.vivid_cmds);
		break;
	default:
		LCD_KIT_INFO("wrong mode!\n");
		ret = LCD_KIT_FAIL;
		break;
	}
	common_info->ce.mode = mode;
	LCD_KIT_INFO("ce.support = %d,ce.mode = %d\n", common_info->ce.support,
		common_info->ce.mode);
	return ret;
}

static int lcd_kit_get_acl_mode(char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->acl.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", common_info->acl.mode);
	return ret;
}

static int lcd_kit_set_acl_mode(void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->acl.support) {
		LCD_KIT_DEBUG("not support acl\n");
		return ret;
	}
	switch (mode) {
	case ACL_OFF_MODE:
		ret = adapt_ops->mipi_tx(hld, &common_info->acl.acl_off_cmds);
		break;
	case ACL_HIGH_MODE:
		ret = adapt_ops->mipi_tx(hld, &common_info->acl.acl_high_cmds);
		break;
	case ACL_MIDDLE_MODE:
		ret = adapt_ops->mipi_tx(hld, &common_info->acl.acl_middle_cmds);
		break;
	case ACL_LOW_MODE:
		ret = adapt_ops->mipi_tx(hld, &common_info->acl.acl_low_cmds);
		break;
	default:
		LCD_KIT_ERR("mode error\n");
		ret = LCD_KIT_FAIL;
		break;
	}
	common_info->acl.mode = mode;
	LCD_KIT_ERR("acl.support = %d,acl.mode = %d\n",
		common_info->acl.support, common_info->acl.mode);
	return ret;
}

static int lcd_kit_get_vr_mode(char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->vr.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", common_info->vr.mode);
	return ret;
}

static int lcd_kit_set_vr_mode(void *hld, u32 mode)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->vr.support) {
		LCD_KIT_DEBUG("not support vr\n");
		return ret;
	}
	switch (mode) {
	case VR_ENABLE:
		ret = adapt_ops->mipi_tx(hld, &common_info->vr.enable_cmds);
		break;
	case  VR_DISABLE:
		ret = adapt_ops->mipi_tx(hld, &common_info->vr.disable_cmds);
		break;
	default:
		ret = LCD_KIT_FAIL;
		LCD_KIT_ERR("mode error\n");
		break;
	}
	common_info->vr.mode = mode;
	LCD_KIT_INFO("vr.support = %d, vr.mode = %d\n", common_info->vr.support,
		common_info->vr.mode);
	return ret;
}

static int lcd_kit_get_effect_color_mode(char *buf)
{
	int ret = LCD_KIT_OK;

	if (common_info->effect_color.support ||
		(common_info->effect_color.mode & BITS(31)))
		ret = snprintf(buf, PAGE_SIZE, "%d\n",
			common_info->effect_color.mode);

	return ret;
}

static int lcd_kit_set_effect_color_mode(u32 mode)
{
	int ret = LCD_KIT_OK;

	if (common_info->effect_color.support)
		common_info->effect_color.mode = mode;
	LCD_KIT_INFO("effect_color.support = %d, effect_color.mode = %d\n",
		common_info->effect_color.support,
		common_info->effect_color.mode);
	return ret;
}

static int lcd_kit_set_mipi_backlight(void *hld, u32 level)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	if (!lcd_ops) {
		LCD_KIT_ERR("lcd_ops is null\n");
		return 0;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	switch (common_info->backlight.order) {
	case BL_BIG_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = (level >> 8) & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = level & 0xFF;
		}
		break;
	case BL_LITTLE_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] = (level >> 8) & 0xFF;
		}
		break;
	default:
		LCD_KIT_ERR("not support order\n");
		break;
	}
	if (common_info->set_vss.support) {
		common_info->set_vss.new_backlight = level;
		if (lcd_ops->set_vss_by_thermal)
			lcd_ops->set_vss_by_thermal();
	}
	ret = adapt_ops->mipi_tx(hld, &common_info->backlight.bl_cmd);

	return ret;
}

static void lcd_kit_panel_parse_effect(struct device_node *np)
{
	/* bl max level */
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&common_info->bl_level_max, 0);
	/* bl min level */
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		 &common_info->bl_level_min, 0);
	/* bl max nit */
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max-nit",
		&common_info->bl_max_nit, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-getblmaxnit-type",
		&common_info->blmaxnit.get_blmaxnit_type, 0);
	lcd_kit_parse_u32(np, "lcd-kit,Does-lcd-poweron-tp",
		&common_info->ul_does_lcd_poweron_tp, 0);
	lcd_kit_parse_u32(np, "lcd-kit,Tp-gesture-sequence-flag",
		&common_info->tp_gesture_sequence_flag, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-on-always-reset",
		&common_info->panel_on_always_need_reset, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-blmaxnit-min-value",
		&common_info->bl_max_nit_min_value, BL_NIT);

	/* get blmaxnit */
	if (common_info->blmaxnit.get_blmaxnit_type == GET_BLMAXNIT_FROM_DDIC)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-bl-maxnit-command",
			"lcd-kit,panel-bl-maxnit-command-state",
			&common_info->blmaxnit.bl_maxnit_cmds);
	/* cabc */
	lcd_kit_parse_u32(np, "lcd-kit,cabc-support",
		&common_info->cabc.support, 0);
	if (common_info->cabc.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-off-cmds",
			"lcd-kit,cabc-off-cmds-state",
			&common_info->cabc.cabc_off_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-ui-cmds",
			"lcd-kit,cabc-ui-cmds-state",
			&common_info->cabc.cabc_ui_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-still-cmds",
			"lcd-kit,cabc-still-cmds-state",
			&common_info->cabc.cabc_still_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,cabc-moving-cmds",
			"lcd-kit,cabc-moving-cmds-state",
			&common_info->cabc.cabc_moving_cmds);
	}
	/* hbm */
	lcd_kit_parse_u32(np, "lcd-kit,hbm-support",
		&common_info->hbm.support, 0);
	if (common_info->hbm.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-enter-no-dim-cmds",
			"lcd-kit,hbm-enter-no-dim-cmds-state",
			&common_info->hbm.enter_no_dim_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-enter-cmds",
			"lcd-kit,hbm-enter-cmds-state",
			&common_info->hbm.enter_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fp-enter-extern-cmds",
			"lcd-kit,fp-enter-extern-cmds-state",
			&common_info->hbm.fp_enter_extern_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fp-exit-extern-cmds",
			"lcd-kit,fp-exit-extern-cmds-state",
			&common_info->hbm.fp_exit_extern_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds",
			"lcd-kit,hbm-prepare-cmds-state",
			&common_info->hbm.hbm_prepare_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-fir",
			"lcd-kit,hbm-prepare-cmds-fir-state",
			&common_info->hbm.prepare_cmds_fir);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-sec",
			"lcd-kit,hbm-prepare-cmds-sec-state",
			&common_info->hbm.prepare_cmds_sec);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-thi",
			"lcd-kit,hbm-prepare-cmds-thi-state",
			&common_info->hbm.prepare_cmds_thi);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-prepare-cmds-fou",
			"lcd-kit,hbm-prepare-cmds-fou-state",
			&common_info->hbm.prepare_cmds_fou);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-fir",
			"lcd-kit,hbm-exit-cmds-fir-state",
			&common_info->hbm.exit_cmds_fir);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-sec",
			"lcd-kit,hbm-exit-cmds-sec-state",
			&common_info->hbm.exit_cmds_sec);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-thi",
			"lcd-kit,hbm-exit-cmds-thi-state",
			&common_info->hbm.exit_cmds_thi);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-thi-new",
			"lcd-kit,hbm-exit-cmds-thi-new-state",
			&common_info->hbm.exit_cmds_thi_new);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds-fou",
			"lcd-kit,hbm-exit-cmds-fou-state",
			&common_info->hbm.exit_cmds_fou);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-cmds",
			"lcd-kit,hbm-cmds-state",
			&common_info->hbm.hbm_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-post-cmds",
			"lcd-kit,hbm-post-cmds-state",
			&common_info->hbm.hbm_post_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-exit-cmds",
			"lcd-kit,hbm-exit-cmds-state",
			&common_info->hbm.exit_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,enter-dim-cmds",
			"lcd-kit,enter-dim-cmds-state",
			&common_info->hbm.enter_dim_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,exit-dim-cmds",
			"lcd-kit,exit-dim-cmds-state",
			&common_info->hbm.exit_dim_cmds);
		lcd_kit_parse_u32(np,
			"lcd-kit,hbm-special-bit-ctrl-support",
			&common_info->hbm.hbm_special_bit_ctrl, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,hbm-set-elvss-dim-lp",
			&common_info->hbm.hbm_set_elvss_dim_lp, 0);
		lcd_kit_parse_u32(np,
			"lcd-kit,hbm-elvss-dim-cmd-delay",
			&common_info->hbm.hbm_fp_elvss_cmd_delay, 0);
		lcd_kit_parse_dcs_cmds(np,
			"lcd-kit,panel-hbm-elvss-prepare-cmds",
			"lcd-kit,panel-hbm-elvss-prepare-cmds-state ",
			&common_info->hbm.elvss_prepare_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-hbm-elvss-read-cmds",
			"lcd-kit,panel-hbm-elvss-read-cmds-state",
			&common_info->hbm.elvss_read_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-hbm-elvss-write-cmds",
			"lcd-kit,panel-hbm-elvss-write-cmds-state",
			&common_info->hbm.elvss_write_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-hbm-elvss-post-cmds",
			"lcd-kit,panel-hbm-elvss-post-cmds-state",
			&common_info->hbm.elvss_post_cmds);
		lcd_kit_parse_u32(np, "lcd-kit,hbm-fp-support",
			&common_info->hbm.hbm_fp_support, 0);
		if (common_info->hbm.hbm_fp_support) {
			lcd_kit_parse_u32(np, "lcd-kit,hbm-level-max",
				&common_info->hbm.hbm_level_max, 0);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-fp-enter-cmds",
				"lcd-kit,hbm-fp-enter-cmds-state",
				&common_info->hbm.fp_enter_cmds);
			lcd_kit_parse_u32(np,
				"lcd-kit,hbm-elvss-dim-support",
				&common_info->hbm.hbm_fp_elvss_support, 0);
		}
		lcd_kit_parse_u32(np, "lcd-kit,lcd-alpha-support",
			&common_info->hbm.local_hbm_support, 0);
		if (common_info->hbm.local_hbm_support) {
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alpha-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_alpha_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alpha-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_alpha_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_circle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_circle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alphacircle-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_alphacircle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alphacircle-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_alphacircle_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-coordinate-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_coordinate_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-size-small-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_size_small_cmds);//local-hbm-fp-circle-radius-cmds
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-size-mid-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_size_mid_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-size-large-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_size_large_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-circle-color-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.circle_color_cmds);
			lcd_kit_parse_array_data(np, "lcd-kit,alpha-table",
				&common_info->hbm.alpha_table);
		}
	}
	/* acl */
	lcd_kit_parse_u32(np, "lcd-kit,acl-support",
		&common_info->acl.support, 0);
	if (common_info->acl.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-enable-cmds",
			"lcd-kit,acl-enable-cmds-state",
			&common_info->acl.acl_enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-high-cmds",
			"lcd-kit,acl-high-cmds-state",
			&common_info->acl.acl_high_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-low-cmds",
			"lcd-kit,acl-low-cmds-state",
			&common_info->acl.acl_low_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-middle-cmds",
			"lcd-kit,acl-middle-cmds-state",
			&common_info->acl.acl_middle_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,acl-off-cmds",
			"lcd-kit,acl-off-cmds-state",
			&common_info->acl.acl_off_cmds);
	}
	/* vr */
	lcd_kit_parse_u32(np, "lcd-kit,vr-support",
		&common_info->vr.support, 0);
	if (common_info->vr.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vr-enable-cmds",
			"lcd-kit,vr-enable-cmds-state",
			&common_info->vr.enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vr-disable-cmds",
			"lcd-kit,vr-disable-cmds-state",
			&common_info->vr.disable_cmds);
	}
	/* ce */
	lcd_kit_parse_u32(np, "lcd-kit,ce-support",
		&common_info->ce.support, 0);
	if (common_info->ce.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-off-cmds",
			"lcd-kit,ce-off-cmds-state",
			&common_info->ce.off_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-srgb-cmds",
			"lcd-kit,ce-srgb-cmds-state",
			&common_info->ce.srgb_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-user-cmds",
			"lcd-kit,ce-user-cmds-state",
			&common_info->ce.user_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,ce-vivid-cmds",
			"lcd-kit,ce-vivid-cmds-state",
			&common_info->ce.vivid_cmds);
	}
	/* effect on */
	lcd_kit_parse_u32(np, "lcd-kit,effect-on-support",
		&common_info->effect_on.support, 0);
	if (common_info->effect_on.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,effect-on-cmds",
			"lcd-kit,effect-on-cmds-state",
			&common_info->effect_on.cmds);
	/* grayscale optimize */
	lcd_kit_parse_u32(np, "lcd-kit,grayscale-optimize-support",
		&common_info->grayscale_optimize.support, 0);
	if (common_info->grayscale_optimize.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,grayscale-optimize-cmds",
			"lcd-kit,grayscale-optimize-cmds-state",
			&common_info->grayscale_optimize.cmds);
}

static void lcd_kit_pcamera_position_para_parse(struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,pre-camera-position-support",
		&common_info->p_cam_position.support, 0);
	if (common_info->p_cam_position.support)
		lcd_kit_parse_u32(np, "lcd-kit,pre-camera-position-end-y",
			&common_info->p_cam_position.end_y, 0);
}

static void lcd_kit_proximity_parse(struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,thp-proximity-support",
		&common_info->thp_proximity.support, 0);
	if (common_info->thp_proximity.support) {
		common_info->thp_proximity.work_status = TP_PROXMITY_DISABLE;
		common_info->thp_proximity.panel_power_state = POWER_ON;
		lcd_kit_parse_u32(np, "lcd-kit,proximity-reset-delay-min",
			&common_info->thp_proximity.after_reset_delay_min, 0);
		memset(&common_info->thp_proximity.lcd_reset_record_tv,
			0, sizeof(struct timeval));
	}
}

static void lcd_kit_panel_parse_util(struct device_node *np)
{
	/* panel name */
	common_info->panel_name = (char *)of_get_property(np,
		"lcd-kit,panel-name", NULL);
	/* panel model */
	common_info->panel_model = (char *)of_get_property(np,
		"lcd-kit,panel-model", NULL);
	/* panel type */
	lcd_kit_parse_u32(np, "lcd-kit,panel-type",
		&common_info->panel_type, 0);
	/* esd */
	lcd_kit_parse_u32(np, "lcd-kit,esd-support",
		&common_info->esd.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-recovery-bl-support",
		&common_info->esd.recovery_bl_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,esd-te-check-support",
		&common_info->esd.te_check_support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,fac-esd-support",
		&common_info->esd.fac_esd_support, 0);
	if (common_info->esd.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,esd-reg-cmds",
			"lcd-kit,esd-reg-cmds-state", &common_info->esd.cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,esd-value",
			&common_info->esd.value);
	}
	lcd_kit_pcamera_position_para_parse(np);

	/* backlight */
	lcd_kit_parse_u32(np, "lcd-kit,backlight-order",
		&common_info->backlight.order, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		&common_info->backlight.bl_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&common_info->backlight.bl_max, 0);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,backlight-cmds",
		"lcd-kit,backlight-cmds-state", &common_info->backlight.bl_cmd);

	/* vss */
	lcd_kit_parse_u32(np, "lcd-kit,vss-support",
		&common_info->set_vss.support, 0);
	if (common_info->set_vss.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vss-cmds-fir",
			"lcd-kit,vss-cmds-fir-state",
			&common_info->set_vss.cmds_fir);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vss-cmds-sec",
			"lcd-kit,vss-cmds-sec-state",
			&common_info->set_vss.cmds_sec);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vss-cmds-thi",
			"lcd-kit,vss-cmds-thi-state",
			&common_info->set_vss.cmds_thi);
	}
	/* check reg on */
	lcd_kit_parse_u32(np, "lcd-kit,check-reg-on-support",
		&common_info->check_reg_on.support, 0);
	if (common_info->check_reg_on.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-reg-on-cmds",
			"lcd-kit,check-reg-on-cmds-state",
			&common_info->check_reg_on.cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,check-reg-on-value",
			&common_info->check_reg_on.value);
		lcd_kit_parse_u32(np,
			"lcd-kit,check-reg-on-support-dsm-report",
			&common_info->check_reg_on.support_dsm_report, 0);
	}
	/* check reg off */
	lcd_kit_parse_u32(np, "lcd-kit,check-reg-off-support",
		&common_info->check_reg_off.support, 0);
	if (common_info->check_reg_off.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,check-reg-off-cmds",
			"lcd-kit,check-reg-off-cmds-state",
			&common_info->check_reg_off.cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,check-reg-off-value",
			&common_info->check_reg_off.value);
		lcd_kit_parse_u32(np,
			"lcd-kit,check-reg-off-support-dsm-report",
			&common_info->check_reg_off.support_dsm_report, 0);
	}
	/* elvdd detect */
	lcd_kit_parse_u32(np, "lcd-kit,elvdd-detect-support",
		&common_info->elvdd_detect.support, NOT_SUPPORT);
	if (common_info->elvdd_detect.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,open-elvdd-detect-cmds",
			"lcd-kit,open-elvdd-detect-cmds-state",
			&common_info->elvdd_detect.cmds);
	/* thp proximity */
	lcd_kit_proximity_parse(np);
	/* sn code */
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-support",
		&common_info->sn_code.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-check",
		&common_info->sn_code.check_support, 0);
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-info",
		&common_info->sn_code.sn_code_info);
	/* fps drf and hbm code */
	lcd_kit_parse_u32(np, "lcd-kit,fps-lock-command-support",
		&common_info->dfr_info.fps_lock_command_support, 0);
	if (common_info->dfr_info.fps_lock_command_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds-dimming",
			"lcd-kit,fps-to-60-cmds-state",
			&common_info->dfr_info.cmds[FPS_60P_NORMAL_DIM]);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds-hbm",
			"lcd-kit,fps-to-60-cmds-state",
			&common_info->dfr_info.cmds[FPS_60P_HBM_NO_DIM]);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-60-cmds-hbm-dimming",
			"lcd-kit,fps-to-60-cmds-state",
			&common_info->dfr_info.cmds[FPS_60P_HBM_DIM]);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds-dimming",
			"lcd-kit,fps-to-90-cmds-state",
			&common_info->dfr_info.cmds[FPS_90_NORMAL_DIM]);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds-hbm",
			"lcd-kit,fps-to-90-cmds-state",
			&common_info->dfr_info.cmds[FPS_90_HBM_NO_DIM]);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds-hbm-dimming",
			"lcd-kit,fps-to-90-cmds-state",
			&common_info->dfr_info.cmds[FPS_90_HBM_DIM]);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,fps-to-90-cmds",
			"lcd-kit,fps-to-90-cmds-state",
			&common_info->dfr_info.cmds[FPS_90_NORMAL_NO_DIM]);
		init_waitqueue_head(&common_info->dfr_info.fps_wait);
		init_waitqueue_head(&common_info->dfr_info.hbm_wait);
	}
	/* aod exit disp on cmds */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-aod-exit-dis-on-cmds",
		"lcd-kit,panel-aod-exit-dis-on-cmds-state",
		&common_info->aod_exit_dis_on_cmds);
}

static void lcd_kit_parse_power_seq(struct device_node *np)
{
	lcd_kit_parse_arrays_data(np, "lcd-kit,power-on-stage",
		&power_seq->power_on_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,lp-on-stage",
		&power_seq->panel_on_lp_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,hs-on-stage",
		&power_seq->panel_on_hs_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,gesture-power-on-stage",
		&power_seq->gesture_power_on_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,power-off-stage",
		&power_seq->power_off_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,lp-off-stage",
		&power_seq->panel_off_lp_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,hs-off-stage",
		&power_seq->panel_off_hs_seq, SEQ_NUM);
	lcd_kit_parse_arrays_data(np, "lcd-kit,only-power-off-stage",
		&power_seq->only_power_off_seq, SEQ_NUM);
}

static void lcd_kit_parse_power(struct device_node *np)
{
	/* vci */
	if (power_hdl->lcd_vci.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vci",
			&power_hdl->lcd_vci);
	/* iovcc */
	if (power_hdl->lcd_iovcc.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-iovcc",
			&power_hdl->lcd_iovcc);
	/* vsp */
	if (power_hdl->lcd_vsp.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vsp",
			&power_hdl->lcd_vsp);
	/* vsn */
	if (power_hdl->lcd_vsn.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vsn",
			&power_hdl->lcd_vsn);
	/* lcd reset */
	if (power_hdl->lcd_rst.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-reset",
			&power_hdl->lcd_rst);
	/* backlight */
	if (power_hdl->lcd_backlight.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-backlight",
			&power_hdl->lcd_backlight);
	/* TE0 */
	if (power_hdl->lcd_te0.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-te0",
			&power_hdl->lcd_te0);
	/* tp reset */
	if (power_hdl->tp_rst.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,tp-reset",
			&power_hdl->tp_rst);
	/* vdd */
	if (power_hdl->lcd_vdd.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-vdd",
			&power_hdl->lcd_vdd);
	if (power_hdl->lcd_aod.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-aod",
			&power_hdl->lcd_aod);
	/* bias */
	if (power_hdl->lcd_power_down_vsp.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-power-down-vsp",
			&power_hdl->lcd_power_down_vsp);
	if (power_hdl->lcd_power_down_vsn.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-power-down-vsn",
			&power_hdl->lcd_power_down_vsn);
	if (power_hdl->lcd_mipi_switch.buf == NULL)
		lcd_kit_parse_array_data(np, "lcd-kit,lcd-mipi-switch",
			&power_hdl->lcd_mipi_switch);
}

static void lcd_kit_parse_regulate(struct device_node *np)
{
	int ret;
	const char *st = NULL;

	/* vci */
	if (power_hdl->lcd_vci.buf &&
		power_hdl->lcd_vci.buf[POWER_MODE] == REGULATOR_MODE) {
		ret = lcd_kit_parse_string(np, "lcd-kit,lcd-vci-name", &st);
		if (ret) {
			LCD_KIT_ERR("failed to read vci name, rc = %d\n", ret);
		} else {
			if (strlen(st) < LCD_KIT_REGULATE_NAME_LEN)
				snprintf(common_info->vci_vreg.name,
					LCD_KIT_REGULATE_NAME_LEN,
					"%s", st);
			else
				LCD_KIT_ERR("vci name len is too long\n");
		}
	}
	/* iovcc */
	if (power_hdl->lcd_iovcc.buf &&
		power_hdl->lcd_iovcc.buf[POWER_MODE] == REGULATOR_MODE) {
		ret = lcd_kit_parse_string(np, "lcd-kit,lcd-iovcc-name", &st);
		if (ret) {
			LCD_KIT_ERR("failed to read iovcc name, rc = %d\n", ret);
		} else {
			if (strlen(st) < LCD_KIT_REGULATE_NAME_LEN)
				snprintf(common_info->iovcc_vreg.name,
			 		LCD_KIT_REGULATE_NAME_LEN,
					 "%s", st);
			else
				LCD_KIT_ERR("iovcc name len is too long\n");
		}
	}
	/* vsp */
	if (power_hdl->lcd_vsp.buf &&
		power_hdl->lcd_vsp.buf[POWER_MODE] == REGULATOR_MODE) {
		ret = lcd_kit_parse_string(np, "lcd-kit,lcd-vsp-name", &st);
		if (ret) {
			LCD_KIT_ERR("failed to read vsp name, rc = %d\n", ret);
		} else {
			if (strlen(st) < LCD_KIT_REGULATE_NAME_LEN)
				snprintf(common_info->vsp_vreg.name,
			 		LCD_KIT_REGULATE_NAME_LEN,
					 "%s", st);
			else
				LCD_KIT_ERR("vsp name len is too long\n");
		}
	}
	/* vsn */
	if (power_hdl->lcd_vsn.buf &&
		power_hdl->lcd_vsn.buf[POWER_MODE] == REGULATOR_MODE) {
		ret = lcd_kit_parse_string(np, "lcd-kit,lcd-vsn-name", &st);
		if (ret) {
			LCD_KIT_ERR("failed to read vsn name, rc = %d\n", ret);
		} else {
			if (strlen(st) < LCD_KIT_REGULATE_NAME_LEN)
				snprintf(common_info->vsn_vreg.name,
			 		LCD_KIT_REGULATE_NAME_LEN,
					 "%s", st);
			else
				LCD_KIT_ERR("vsn name len is too long\n");
		}
	}
}

static int lcd_kit_panel_parse_dt(struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("np is null\n");
		return LCD_KIT_FAIL;
	}
	/* parse effect info */
	lcd_kit_panel_parse_effect(np);
	/* parse normal info */
	lcd_kit_panel_parse_util(np);
	/* parse power sequence */
	lcd_kit_parse_power_seq(np);
	/* parse power */
	lcd_kit_parse_power(np);
	lcd_kit_parse_regulate(np);
	return LCD_KIT_OK;
}

static int lcd_kit_get_bias_voltage(int *vpos, int *vneg)
{
	if (!vpos || !vneg) {
		LCD_KIT_ERR("vpos/vneg is null\n");
		return LCD_KIT_FAIL;
	}
	if (power_hdl->lcd_vsp.buf)
		*vpos = power_hdl->lcd_vsp.buf[POWER_VOL];
	if (power_hdl->lcd_vsn.buf)
		*vneg = power_hdl->lcd_vsn.buf[POWER_VOL];
	return LCD_KIT_OK;
}

static void lcd_kit_lock_init(void)
{
	/* init mipi lock */
	mutex_init(&COMMON_LOCK->mipi_lock);
	if (common_info->hbm.support)
		mutex_init(&COMMON_LOCK->hbm_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.model_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.type_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.panel_info_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.vol_enable_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.amoled_acl_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.amoled_vr_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.support_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.gamma_dynamic_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.frame_count_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.frame_update_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.mipi_dsi_clk_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.fps_scence_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.fps_order_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.alpm_function_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.alpm_setting_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.ddic_alpha_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.func_switch_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.reg_read_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.ddic_oem_info_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.bl_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.support_bl_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.effect_bl_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.ddic_lv_detect_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.hbm_mode_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.panel_sn_code_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.pre_camera_position_lock);
	mutex_init(&COMMON_LOCK->sysfs_lock.cabc_mode_lock);
}
static int lcd_kit_common_init(struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node!\n");
		return LCD_KIT_FAIL;
	}

	lcd_kit_panel_parse_dt(np);
	lcd_kit_lock_init();
	return LCD_KIT_OK;
}

#ifdef CONFIG_HUAWEI_DSM
int lcd_dsm_client_record(struct dsm_client *lcd_dclient, char *record_buf,
	int lcd_dsm_error_no, int rec_num_limit, int *cur_rec_time)
{
	if (!lcd_dclient || !record_buf || !cur_rec_time) {
		LCD_KIT_ERR("null pointer!\n");
		return LCD_KIT_FAIL;
	}

	if ((rec_num_limit >= 0) && (*cur_rec_time > rec_num_limit)) {
		LCD_KIT_INFO("dsm record limit!\n");
		return LCD_KIT_OK;
	}

	if (!dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, record_buf);
		dsm_client_notify(lcd_dclient, lcd_dsm_error_no);
		(*cur_rec_time)++;
		return LCD_KIT_OK;
	}
	LCD_KIT_ERR("dsm_client_ocuppy failed!\n");
	return LCD_KIT_FAIL;
}
#endif

void lcd_kit_delay(int wait, int waittype, bool allow_sleep)
{
	if (!wait) {
		LCD_KIT_DEBUG("wait is 0\n");
		return;
	}
	if (waittype == LCD_KIT_WAIT_US) {
		udelay(wait);
	} else if (waittype == LCD_KIT_WAIT_MS) {
		if (wait > 10 && allow_sleep)
			usleep_range(wait * 1000, wait * 1000);
		else
			mdelay(wait);
	} else {
		if (allow_sleep)
			msleep(wait * 1000);
		else
			mdelay(wait * 1000);
	}
}

/* common ops */
struct lcd_kit_common_ops g_lcd_kit_common_ops = {
	.common_init = lcd_kit_common_init,
	.panel_power_on = lcd_kit_panel_power_on,
	.panel_on_lp = lcd_kit_panel_on_lp,
	.panel_on_hs = lcd_kit_panel_on_hs,
	.panel_off_hs = lcd_kit_panel_off_hs,
	.panel_off_lp = lcd_kit_panel_off_lp,
	.panel_power_off = lcd_kit_panel_power_off,
	.panel_only_power_off = lcd_kit_panel_only_power_off,
	.get_panel_name = lcd_kit_get_panel_name,
	.get_panel_info = lcd_kit_get_panel_info,
	.get_cabc_mode = lcd_kit_get_cabc_mode,
	.set_cabc_mode = lcd_kit_set_cabc_mode,
	.get_acl_mode = lcd_kit_get_acl_mode,
	.set_acl_mode = lcd_kit_set_acl_mode,
	.get_vr_mode = lcd_kit_get_vr_mode,
	.set_vr_mode = lcd_kit_set_vr_mode,
	.esd_handle = lcd_kit_esd_handle,
	.set_ce_mode = lcd_kit_set_ce_mode,
	.get_ce_mode = lcd_kit_get_ce_mode,
	.hbm_set_handle = lcd_kit_hbm_set_handle,
	.fp_hbm_enter_extern = lcd_kit_enter_fp_hbm_extern,
	.fp_hbm_exit_extern = lcd_kit_exit_fp_hbm_extern,
	.set_ic_dim_on = lcd_kit_hbm_dim_enable,
	.set_effect_color_mode = lcd_kit_set_effect_color_mode,
	.get_effect_color_mode = lcd_kit_get_effect_color_mode,
	.set_mipi_backlight = lcd_kit_set_mipi_backlight,
	.get_bias_voltage = lcd_kit_get_bias_voltage,
};
