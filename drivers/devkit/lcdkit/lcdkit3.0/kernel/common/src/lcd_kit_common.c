/*
 * lcd_kit_common.c
 *
 * lcdkit common function for lcdkit
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

#include "lcd_kit_common.h"
#include "lcd_kit_dbg.h"
#include "lcd_kit_parse.h"
#if defined(CONFIG_LCD_KIT_HISI)
#include "lcd_kit_ext_disp.h"
#include "hisi_fb.h"
#else
#define LCD_ACTIVE_PANEL_BUTT 1
#define LCD_ACTIVE_PANEL 0
#define LCD_FOLDER_TYPE 1
#endif

#if defined CONFIG_HUAWEI_DSM
extern struct dsm_client *lcd_dclient;
#endif
#ifdef LV_GET_LCDBK_ON
u32 mipi_level;
#endif

#define BL_MAX 256
#define SKIP_SEND_EVENT 1
#define MAX_BTB_IRQ_COUNT 2
#define ESD_DETECT_GPIO_ONLY_MODE 2
static int btb_irq_count = 0;
static int lcd_backlight_i2c_count = 0;
static int lcd_bias_i2c_count = 0;
static const uint32_t fps_normal = 60;
static const uint32_t fps_medium = 90;

/* The maximum number of enter_ddic_alpha commands is 16, you can change it if needed */
static const int cmd_cnt_max = 16;
static const int payload_num_min = 3;
static const int payload_num_max = 8;

enum hbm_gamma_index {
	GAMMA_INDEX_RED_HIGH = 0,
	GAMMA_INDEX_RED_LOW = 1,
	GAMMA_INDEX_GREEN_HIGH = 24,
	GAMMA_INDEX_GREEN_LOW = 25,
	GAMMA_INDEX_BLUE_HIGH = 48,
	GAMMA_INDEX_BLUE_LOW = 49
};

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

struct lcd_kit_power_desc *lcd_kit_get_panel_power_handle(
	uint32_t panel_id)
{
	if (panel_id >= LCD_ACTIVE_PANEL_BUTT) {
		LCD_KIT_ERR("panel_id %u!\n", panel_id);
		return NULL;
	}
	return &g_lcd_kit_power_handle[panel_id];
}

struct lcd_kit_power_seq *lcd_kit_get_panel_power_seq(
	uint32_t panel_id)
{
	if (panel_id >= LCD_ACTIVE_PANEL_BUTT) {
		LCD_KIT_ERR("panel_id %u!\n", panel_id);
		return NULL;
	}
	return &g_lcd_kit_power_seq[panel_id];
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
	/* init code */
	if (adapt_ops->mipi_tx) {
		ret = adapt_ops->mipi_tx(hld, &common_info->panel_on_cmds);
		if (ret)
			LCD_KIT_ERR("send panel on cmds error\n");
		/* send panel on effect code */
		if (!common_info->effect_on.support)
			return ret;
		ret = adapt_ops->mipi_tx(hld, &common_info->effect_on.cmds);
		if (ret)
			LCD_KIT_ERR("send effect on cmds error\n");
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
	/* send panel off code */
	if (adapt_ops->mipi_tx)
		ret = adapt_ops->mipi_tx(hld, &common_info->panel_off_cmds);
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

static void lcd_kit_proxmity_proc(int enable)
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
		if (common_info->aod_no_need_init &&
			common_info->new_doze_state) {
			common_info->new_doze_state = false;
			LCD_KIT_INFO("aod no need init\n");
			return ret;
		}
		lcd_kit_proxmity_proc(enable);
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
#if defined(CONFIG_LCD_KIT_HISI)
static int lcd_kit_ext_ts_resume(int sync)
{
	int ret;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_multi_power_notify(TS_RESUME_DEVICE,
			SHORT_SYNC, LCD_ACTIVE_PANEL);
	else
		ret = ts_ops->ts_multi_power_notify(TS_RESUME_DEVICE,
			NO_SYNC, LCD_ACTIVE_PANEL);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}
#endif
static int lcd_kit_ts_resume(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

#if defined(CONFIG_LCD_KIT_HISI)
	if (lcd_kit_get_product_type() == LCD_FOLDER_TYPE)
		return lcd_kit_ext_ts_resume(sync);
#endif

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
#if defined(CONFIG_LCD_KIT_HISI)
static int lcd_kit_ext_ts_after_resume(int sync)
{
	int ret;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_multi_power_notify(TS_AFTER_RESUME,
			SHORT_SYNC, LCD_ACTIVE_PANEL);
	else
		ret = ts_ops->ts_multi_power_notify(TS_AFTER_RESUME,
			NO_SYNC, LCD_ACTIVE_PANEL);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}
#endif
static int lcd_kit_ts_after_resume(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

#if defined(CONFIG_LCD_KIT_HISI)
	if (lcd_kit_get_product_type() == LCD_FOLDER_TYPE)
		return lcd_kit_ext_ts_after_resume(sync);
#endif

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
#if defined(CONFIG_LCD_KIT_HISI)
static int lcd_kit_ext_ts_suspend(int sync)
{
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync) {
		ts_ops->ts_multi_power_notify(TS_BEFORE_SUSPEND,
			SHORT_SYNC, LCD_ACTIVE_PANEL);
		ts_ops->ts_multi_power_notify(TS_SUSPEND_DEVICE,
			SHORT_SYNC, LCD_ACTIVE_PANEL);
	} else {
		ts_ops->ts_multi_power_notify(TS_BEFORE_SUSPEND,
			NO_SYNC, LCD_ACTIVE_PANEL);
		ts_ops->ts_multi_power_notify(TS_SUSPEND_DEVICE,
			NO_SYNC, LCD_ACTIVE_PANEL);
	}
	LCD_KIT_INFO("sync is %d\n", sync);
	return LCD_KIT_OK;
}
#endif
static int lcd_kit_ts_suspend(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

#if defined(CONFIG_LCD_KIT_HISI)
	if (lcd_kit_get_product_type() == LCD_FOLDER_TYPE)
		return lcd_kit_ext_ts_suspend(sync);
#endif

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
#if defined(CONFIG_LCD_KIT_HISI)
static int lcd_kit_ext_ts_early_suspend(int sync)
{
	int ret;
	struct ts_kit_ops *ts_ops = NULL;

	ts_ops = ts_kit_get_ops();
	if (!ts_ops || !ts_ops->ts_multi_power_notify) {
		LCD_KIT_ERR("ts_ops or ts_power_notify is null\n");
		return LCD_KIT_FAIL;
	}
	if (sync)
		ret = ts_ops->ts_multi_power_notify(TS_EARLY_SUSPEND,
			SHORT_SYNC, LCD_ACTIVE_PANEL);
	else
		ret = ts_ops->ts_multi_power_notify(TS_EARLY_SUSPEND,
			NO_SYNC, LCD_ACTIVE_PANEL);
	LCD_KIT_INFO("sync is %d\n", sync);
	return ret;
}
static bool lcd_kit_is_power_event(uint32_t event)
{
	switch (event) {
	case EVENT_VCI:
	case EVENT_IOVCC:
	case EVENT_RESET:
	case EVENT_VDD:
		return true;
	default:
		break;
	}
	return false;
}
#endif
static int lcd_kit_ts_early_suspend(int sync)
{
	int ret = LCD_KIT_OK;
	struct ts_kit_ops *ts_ops = NULL;

#if defined(CONFIG_LCD_KIT_HISI)
	if (lcd_kit_get_product_type() == LCD_FOLDER_TYPE)
		return lcd_kit_ext_ts_early_suspend(sync);
#endif

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
#if defined(CONFIG_LCD_KIT_HISI)
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	hisifd = (struct hisi_fb_data_type *)hld;
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null!\n");
		return false;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo) {
		LCD_KIT_ERR("panel_info is NULL!\n");
		return false;
	}
	if (pinfo->product_type != LCD_FOLDER_TYPE)
		return false;
	if ((pinfo->skip_power_on_off == FOLD_POWER_ON_OFF) &&
		(enable == 0)) {
		if (lcd_kit_is_power_event(event)) {
			LCD_KIT_INFO("skip power off!\n");
			return true;
		}
	}
	if (pinfo->skip_power_on_off != SKIP_POWER_ON_OFF)
		return false;
	/* mipi switch always need */
	if (event == EVENT_MIPI_SWITCH)
		return false;
	/* only read lcd status reg */
	if (event == EVENT_MIPI) {
		if (enable)
			(void)lcd_kit_check_reg_report_dsm(hld,
				&common_info->check_reg_on);
		else
			(void)lcd_kit_check_reg_report_dsm(hld,
				&common_info->check_reg_off);
	}

	LCD_KIT_INFO("skip power on off!\n");
	return true;
#else
	return false;
#endif
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

	if (lcd_kit_event_skip_send(hld, event, data))
		return SKIP_SEND_EVENT;

	if ((event == EVENT_IOVCC) && (data != 0)) {
		ret = lcd_kit_ft_gesture_mode_power_on();
		if (ret) {
			LCD_KIT_INFO("It is in gesture mode\n");
			return ret;
		}
	}

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

static int lcd_kit_event_handler_part(void *hld, uint32_t event, uint32_t data,
	uint32_t delay, bool before_mipi)
{
	int ret = LCD_KIT_OK;

	switch (event) {
	case EVENT_VCI:
	case EVENT_IOVCC:
	case EVENT_VSP:
	case EVENT_VSN:
	case EVENT_VDD:
	case EVENT_BIAS:
		if (before_mipi)
			ret = lcd_kit_event_handler(hld, event, data, delay);
		break;
	case EVENT_RESET:
	case EVENT_MIPI:
	case EVENT_EARLY_TS:
	case EVENT_LATER_TS:
	case EVENT_AOD:
	case EVENT_AOD_MIPI:
	case EVENT_MIPI_SWITCH:
	case EVENT_AVDD_MIPI:
	case EVENT_2ND_POWER_OFF_TS:
		if (before_mipi == false)
			ret = lcd_kit_event_handler(hld, event, data, delay);
		break;
	case EVENT_NONE:
	default:
		ret = lcd_kit_event_handler(hld, event, data, delay);
		break;
	}
	return ret;
}

static int lcd_kit_event_handler_with_mipi(void *hld, uint32_t event, uint32_t data,
	uint32_t delay)
{
	return lcd_kit_event_handler_part(hld, event, data, delay, false);
}

static int lcd_kit_event_handler_without_mipi(void *hld, uint32_t event, uint32_t data,
	uint32_t delay)
{
	return lcd_kit_event_handler_part(hld, event, data, delay, true);
}

static int lcd_kit_panel_power_on_with_mipi(void *hld)
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
		ret = lcd_kit_event_handler_with_mipi(hld, pevent->buf[EVENT_NUM],
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

static int lcd_kit_panel_power_on_without_mipi(void *hld)
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
		ret = lcd_kit_event_handler_without_mipi(hld, pevent->buf[EVENT_NUM],
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
	/* restart check thread */
	if (common_info->check_thread.enable)
		hrtimer_start(&common_info->check_thread.hrtimer,
			ktime_set(CHECK_THREAD_TIME_PERIOD / 1000, // change ms to s
			(CHECK_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
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
	/* stop check thread */
	if (common_info->check_thread.enable)
		hrtimer_cancel(&common_info->check_thread.hrtimer);
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
	if (common_info->set_power.support)
		common_info->set_power.get_thermal = 0;
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
	/* enable fp hbm */
	if (common_info->hbm.fp_enter_extern_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.fp_enter_extern_cmds);
		LCD_KIT_INFO("fp hbm enter extern cmd already send, ret = %d!\n", ret);
	}
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
	/* recover when exit fp hbm */
	if (common_info->hbm.fp_exit_extern_cmds.cmds != NULL) {
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.fp_exit_extern_cmds);
		LCD_KIT_INFO("fp hbm exit extern cmd already send, ret = %d!\n", ret);
	}
	return ret;
}

void lcd_kit_hbm_fps_handler(int level)
{
	/* Skip unsmooth hbm levels */
	LCD_KIT_DEBUG("level_in=%d!\n", level);
	level = ((unsigned int)level > common_info->hbm.hbm_level_skip_low &&
		(unsigned int)level < common_info->hbm.hbm_level_skip_high) ?
		common_info->hbm.hbm_level_skip_low : level;
	LCD_KIT_DEBUG("level_out=%d!\n", level);
	/* Set hbm level cmd for 120Hz */
	common_info->hbm.hbm_cmds.cmds[3].payload[1] = (level >> 8) & 0xf;
	common_info->hbm.hbm_cmds.cmds[3].payload[2] = level & 0xff;
	/* Set hbm level cmd for 60Hz, hbm level = level << base - bias */
	common_info->hbm.hbm_cmds.cmds[1].payload[1] = (((level <<
		common_info->hbm.hbm_level_base) - (common_info->hbm.hbm_level_bias <<
		common_info->hbm.hbm_level_base)) >> 8) & 0xf;
	common_info->hbm.hbm_cmds.cmds[1].payload[2] = ((level <<
		common_info->hbm.hbm_level_base) - (common_info->hbm.hbm_level_bias <<
		common_info->hbm.hbm_level_base)) & 0xff;
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
		} else if (common_info->hbm.hbm_fps_command_support) {
			/* Send different hbm levels at different freq and skip
			unsmooth hbm levels for wagner product */
			lcd_kit_hbm_fps_handler(level);
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
	/* open dimming exit hbm */
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

static void lcd_kit_clear_esd_error_info(void)
{
	memset(&g_esd_error_info, 0, sizeof(g_esd_error_info));
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

int lcd_kit_judge_esd(unsigned char type, unsigned char read_val,
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

int32_t lcd_kit_get_gpio_value(u32 gpio_num, const char *gpio_name)
{
	int32_t gpio_value;
	int32_t ret;

	ret = gpio_request(gpio_num, gpio_name);
	if (ret != 0) {
		LCD_KIT_ERR("esd_detect_gpio[%d] request fail!\n", gpio_num);
		return LCD_KIT_FAIL;
	}
	ret = gpio_direction_input(gpio_num);
	if (ret != 0) {
		gpio_free(gpio_num);
		LCD_KIT_ERR("esd_detect_gpio[%d] direction set fail!\n", gpio_num);
		return LCD_KIT_FAIL;
	}
	gpio_value = gpio_get_value(gpio_num);
	gpio_free(gpio_num);

	LCD_KIT_INFO("gpio_num:%d value:%d\n", gpio_num, gpio_value);
	return gpio_value;
}

static int lcd_kit_gpio_detect_event(void)
{
	int ret = LCD_KIT_OK;
	int i;
	u32 gpio_detect_value;
	struct ts_kit_ops *ts_ops = NULL;

	if (common_info->esd.tp_esd_event) {
		ts_ops = ts_kit_get_ops();
		if (!ts_ops || !ts_ops->send_esd_event) {
			LCD_KIT_ERR("ts_ops or send_esd_event is null\n");
			return LCD_KIT_FAIL;
		}
	}
	for (i = 0; i < GPIO_CHECK_TIMES; i++) {
		gpio_detect_value = (u32)lcd_kit_get_gpio_value(
			common_info->esd.gpio_detect_num, "esd_detect_gpio");
		if (gpio_detect_value == common_info->esd.gpio_normal_value) {
			break;
		} else if (!common_info->esd.tp_esd_event) {
			return LCD_KIT_FAIL;
		} else if (i == (GPIO_CHECK_TIMES - 1)) {
			ret = ts_ops->send_esd_event(gpio_detect_value);
			if (ret)
				LCD_KIT_ERR("ts_ops->send_esd_event faile\n");
			return LCD_KIT_FAIL;
		}
	}
	return LCD_KIT_OK;
}

static int lcd_kit_esd_handle(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	char read_value[MAX_REG_READ_COUNT] = {0};
	char expect_value, judge_type;
	u32 *esd_value = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	int clear_esd_info_flag = FALSE;

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
	if (common_info->esd.gpio_detect_support) {
		if (lcd_kit_gpio_detect_event())
			return LCD_KIT_FAIL;

		/* esd detect gpio only */
		if (common_info->esd.gpio_detect_support == ESD_DETECT_GPIO_ONLY_MODE)
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
			if (clear_esd_info_flag == FALSE) {
				lcd_kit_clear_esd_error_info();
				clear_esd_info_flag = TRUE;
			}
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

static int lcd_kit_dsi_handle(void *hld)
{
	int ret = LCD_KIT_OK;
	int i;
	char expect_value;
	u32 *dsi_value = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	char read_value[MAX_REG_READ_COUNT] = {0};

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_rx!\n");
		return LCD_KIT_FAIL;
	}
	if (!common_info->dsi.support) {
		LCD_KIT_DEBUG("not support dsi\n");
		return ret;
	}

	dsi_value = common_info->dsi.value.buf;
	ret = adapt_ops->mipi_rx(hld, read_value, MAX_REG_READ_COUNT - 1,
		&common_info->dsi.cmds);
	if (ret) {
		LCD_KIT_INFO("mipi_rx fail\n");
		return ret;
	}

	for (i = 0; i < common_info->dsi.value.cnt; i++) {
		expect_value = dsi_value[i] & 0xFF;
		if (read_value[i] != expect_value) {
			LCD_KIT_ERR("read_value[%d] = 0x%x, expect_value = 0x%x\n",
				i, read_value[i], expect_value);
			ret = snprintf(record_buf_dsi, RECORD_BUFLEN_DSI,
				"dsi read_value = 0x%x, 0x%x", read_value[0], read_value[1]);
			if (ret < 0) {
				LCD_KIT_ERR("snprintf happened error!\n");
				continue;
			}
#ifdef CONFIG_HUAWEI_DSM
			(void)lcd_dsm_client_record(lcd_dclient, record_buf_dsi,
				DSM_DSI_DETECT_ERROR_NO, REC_DMD_NO_LIMIT_DSI, &cur_rec_time_dsi);
#endif
			ret = 1;
			break;
		}
		LCD_KIT_INFO("dsi_value[%d] = 0x%x, read_value = 0x%x\n",
			i, dsi_value[i], read_value[i]);
	}
	LCD_KIT_INFO("dsi check result:%d\n", ret);
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
	if (common_info->backlight.need_prepare)
		ret = adapt_ops->mipi_tx(hld, &common_info->backlight.prepare);
	ret = adapt_ops->mipi_tx(hld, &common_info->backlight.bl_cmd);
	if ((common_info->bl_set_delay > 0) && (level == 0)) {
		LCD_KIT_INFO("set backlight delay %dms while level = %d\n", common_info->bl_set_delay, level);
		lcd_kit_delay(common_info->bl_set_delay, LCD_KIT_WAIT_MS, true);
	}
#ifdef LV_GET_LCDBK_ON
	LCD_KIT_INFO("mipi bl_level = %d\n", level);
	mipi_level = level;
#endif
	return ret;
}

static int lcd_kit_dirty_region_handle(void *hld, struct region_rect *dirty)
{
	int ret = LCD_KIT_OK;
	struct region_rect *dirty_region = NULL;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops or mipi_tx!\n");
		return LCD_KIT_FAIL;
	}
	if (dirty == NULL) {
		LCD_KIT_ERR("dirty is null point!\n");
		return LCD_KIT_FAIL;
	}
	if (common_info->dirty_region.support) {
		dirty_region = (struct region_rect *)dirty;
		/* change region to dsi cmds */
		common_info->dirty_region.cmds.cmds[0].payload[1] =
			((dirty_region->x) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[0].payload[2] =
			(dirty_region->x) & 0xff;
		common_info->dirty_region.cmds.cmds[0].payload[3] =
			((dirty_region->x + dirty_region->w - 1) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[0].payload[4] =
			(dirty_region->x + dirty_region->w - 1) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[1] =
			((dirty_region->y) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[2] =
			(dirty_region->y) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[3] =
			((dirty_region->y + dirty_region->h - 1) >> 8) & 0xff;
		common_info->dirty_region.cmds.cmds[1].payload[4] =
			(dirty_region->y + dirty_region->h - 1) & 0xff;
		ret = adapt_ops->mipi_tx(hld, &common_info->dirty_region.cmds);
	}
	return ret;
}

void lcd_hardware_reset(void)
{
	/* reset pull low */
	lcd_kit_reset_power_ctrl(LCD_RESET_LOW);
	msleep(300);
	/* reset pull high */
	lcd_kit_reset_power_ctrl(LCD_RESET_HIGH);
}

static void lcd_kit_panel_parse_effect(struct device_node *np)
{
	/* effect color */
	lcd_kit_parse_u32(np, "lcd-kit,effect-color-support",
		&common_info->effect_color.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,effect-color-mode",
		&common_info->effect_color.mode, 0);
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
	/* backlight delay */
	lcd_kit_parse_u32(np, "lcd-kit,backlight-set-delay",
		&common_info->bl_set_delay, 0);

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
	/* ddic alpha */
	lcd_kit_parse_u32(np, "lcd-kit,alpha-support",
		&common_info->ddic_alpha.alpha_support, 0);
	if (common_info->ddic_alpha.alpha_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-ddic-alpha-enter-cmds",
			"lcd-kit,local-fp-local-ddic-alpha-cmds-state",
			&common_info->ddic_alpha.enter_alpha_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-ddic-alpha-exit-cmds",
			"lcd-kit,local-fp-local-ddic-alpha-cmds-state",
			&common_info->ddic_alpha.exit_alpha_cmds);
	}
	/* alpha with enable flag */
	lcd_kit_parse_u32(np, "lcd-kit,alpha-with-enable-flag",
		&common_info->ddic_alpha.alpha_with_enable_flag, 0);
	/* force delta bl update support */
	lcd_kit_parse_u32(np, "lcd-kit,force-delta-bl-update-support",
		&common_info->force_delta_bl_update_support, 0);
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
		lcd_kit_parse_array_data(np, "lcd-kit,node-grayscale", &common_info->hbm.gamma_info.node_grayscale);

		lcd_kit_parse_u32(np, "lcd-kit,lcd-alpha-support",
			&common_info->hbm.local_hbm_support, 0);
		if (common_info->hbm.local_hbm_support) {
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alpha-enter-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.enter_alpha_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-fp-alpha-exit-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.exit_alpha_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-dbv-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.hbm_dbv_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-em-60hz-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.hbm_em_configure_60hz_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,local-hbm-em-90hz-cmds",
				"lcd-kit,local-hbm-fp-local-hbm-cmds-state",
				&common_info->hbm.hbm_em_configure_90hz_cmds);
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
				&common_info->hbm.circle_size_small_cmds); // local-hbm-fp-circle-radius-cmds
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
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-60-hz-gamma-read-cmds",
				"lcd-kit,hbm-60-hz-gamma-read-cmds-state",
				&common_info->hbm.hbm_60_hz_gamma_read_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-90-hz-gamma-read-cmds",
				"lcd-kit,hbm-90-hz-gamma-read-cmds-state",
				&common_info->hbm.hbm_90_hz_gamma_read_cmds);
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,hbm-circle-color-setting-cmds",
				"lcd-kit,hbm-circle-color-setting-cmds-state",
				&common_info->hbm.hbm_circle_color_setting_cmds);
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
	/* screen on default effect */
	lcd_kit_parse_u32(np, "lcd-kit,screen-on-effect-support",
		&common_info->screen_on_effect.support, 0);
	if (common_info->screen_on_effect.support) {
		lcd_kit_parse_dcs_cmds(np,
			"lcd-kit,screen-on-effect-prepare-cmds",
			"lcd-kit,screen-on-effect-prepare-cmds-state",
			&common_info->screen_on_effect.prepare_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,screen-on-effect-cmds",
			"lcd-kit,screen-on-effect-cmds-state",
			&common_info->screen_on_effect.cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,screen-on-effect-exit-cmds",
			"lcd-kit,screen-on-effect-exit-cmds-state",
			&common_info->screen_on_effect.exit_cmds);
	}
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
	int32_t gpio_val;

	/* panel name */
	common_info->panel_name = (char *)of_get_property(np,
		"lcd-kit,panel-name", NULL);
	/* panel model */
	common_info->panel_model = (char *)of_get_property(np,
		"lcd-kit,panel-model", NULL);
	/* panel information */
	common_info->module_info = (char *)of_get_property(np,
		"lcd-kit,module-info", NULL);
	/* panel type */
	lcd_kit_parse_u32(np, "lcd-kit,panel-type",
		&common_info->panel_type, 0);
	/* panel on command */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-cmds",
		"lcd-kit,panel-on-cmds-state", &common_info->panel_on_cmds);
	/* panel off command */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-off-cmds",
		"lcd-kit,panel-off-cmds-state", &common_info->panel_off_cmds);
	/* panel on backup */
	lcd_kit_parse_u32(np, "lcd-kit,pnl-on-cmd-bak-support",
		&common_info->panel_cmd_backup.panel_on_support, 0);
	if (common_info->panel_cmd_backup.panel_on_support) {
		lcd_kit_parse_u32(np, "lcd-kit,pnl-cmd-bak-det-gpio",
			&common_info->panel_cmd_backup.detect_gpio, 0);
		lcd_kit_parse_u32(np, "lcd-kit,pnl-cmd-bak-gpio-exp-val",
			&common_info->panel_cmd_backup.gpio_exp_val, 0);
		gpio_val = lcd_kit_get_gpio_value(
			common_info->panel_cmd_backup.detect_gpio,
			"panel_cmd_backup");
		common_info->panel_cmd_backup.change_flag = 0;
		if (common_info->panel_cmd_backup.gpio_exp_val == gpio_val) {
			LCD_KIT_INFO("panel_cmd_backup gpio %d\n", gpio_val);
			/* store change status */
			common_info->panel_cmd_backup.change_flag = 1;
			lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-on-cmds-backup",
				"lcd-kit,panel-on-cmds-state", &common_info->panel_on_cmds);
		}
	}
	/* esd */
	lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-detect-support",
		&common_info->esd.gpio_detect_support, 0);
	if (common_info->esd.gpio_detect_support) {
		lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-detect-num",
			&common_info->esd.gpio_detect_num, 0);
		lcd_kit_parse_u32(np, "lcd-kit,esd-gpio-normal-value",
			&common_info->esd.gpio_normal_value, 0);
		lcd_kit_parse_u32(np, "lcd-kit,tp-esd-gpio-event",
			&common_info->esd.tp_esd_event, 0);
		lcd_kit_parse_u32(np, "lcd-kit,tp-report-detect-times",
			&common_info->esd.tp_report_detect_times, 0);
	}
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
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,esd-reg-ext-cmds",
			"lcd-kit,esd-reg-cmds-state", &common_info->esd.ext_cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,esd-ext-value",
			&common_info->esd.ext_value);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,esd-mipi-err-cmds",
			"lcd-kit,esd-reg-cmds-state", &common_info->esd.mipi_err_cmds);
		lcd_kit_parse_u32(np, "lcd-kit,esd-mipi-err-cnt",
			&common_info->esd.mipi_err_cnt, 0);
	}
	lcd_kit_pcamera_position_para_parse(np);

	/* dsi detect */
	lcd_kit_parse_u32(np, "lcd-kit,dsi-support",
		&common_info->dsi.support, 0);
	if (common_info->dsi.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,dsi-reg-cmds",
			"lcd-kit,dsi-reg-cmds-state", &common_info->dsi.cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,dsi-value",
			&common_info->dsi.value);
	}

	/* dirty region */
	lcd_kit_parse_u32(np, "lcd-kit,dirty-region-support",
		&common_info->dirty_region.support, 0);
	if (common_info->dirty_region.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,dirty-region-cmds",
			"lcd-kit,dirty-region-cmds-state",
			&common_info->dirty_region.cmds);
	/* backlight */
	lcd_kit_parse_u32(np, "lcd-kit,backlight-order",
		&common_info->backlight.order, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-min",
		&common_info->backlight.bl_min, 0);
	lcd_kit_parse_u32(np, "lcd-kit,panel-bl-max",
		&common_info->backlight.bl_max, 0);
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,backlight-cmds",
		"lcd-kit,backlight-cmds-state", &common_info->backlight.bl_cmd);
	lcd_kit_parse_u32(np, "lcd-kit,bl-need-prepare",
		&common_info->backlight.need_prepare, 0);
	if (common_info->backlight.need_prepare)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,bl-prepare-cmds",
			"lcd-kit,bl-prepare-cmds-state",
			&common_info->backlight.prepare);

	/* check thread */
	lcd_kit_parse_u32(np, "lcd-kit,check-thread-enable",
		&common_info->check_thread.enable, 0);
	if (common_info->check_thread.enable)
		lcd_kit_parse_u32(np, "lcd-kit,check-bl-support",
			&common_info->check_thread.check_bl_support, 0);
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
	/* power */
	if (!common_info->set_power.support)
		lcd_kit_parse_u32(np, "lcd-kit,power-support",
			&common_info->set_power.support, 0);

	if (common_info->set_power.support) {
		common_info->set_power.get_thermal = 0;
		lcd_kit_parse_u32(np, "lcd-kit,power-thermal1",
			&common_info->set_power.thermal1, 0);
		lcd_kit_parse_u32(np, "lcd-kit,power-thermal2",
			&common_info->set_power.thermal2, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,power-cmds-fir",
			"lcd-kit,vss-cmds-fir-state",
			&common_info->set_power.cmds_fir);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,power-cmds-sec",
			"lcd-kit,vss-cmds-sec-state",
			&common_info->set_power.cmds_sec);
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
	/* check mipi */
	lcd_kit_parse_u32(np, "lcd-kit,mipi-check-support",
		&common_info->mipi_check.support, 0);
	if (common_info->mipi_check.support) {
		lcd_kit_parse_u32(np,
		"lcd-kit,panel-mipi-error-report-threshold",
		&common_info->mipi_check.mipi_error_report_threshold, 1);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,mipi-check-cmds",
			"lcd-kit,mipi-check-cmds-state",
			&common_info->mipi_check.cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,mipi-check-value",
			&common_info->mipi_check.value);
	}
	/* thp proximity */
	lcd_kit_proximity_parse(np);
	/* sn code */
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-support",
		&common_info->sn_code.support, 0);
	lcd_kit_parse_u32(np, "lcd-kit,sn-code-check",
		&common_info->sn_code.check_support, 0);
	lcd_kit_parse_array_data(np, "lcd-kit,lcd-info",
		&common_info->sn_code.sn_code_info);
	lcd_kit_parse_u32(np, "lcd-kit,hbm-fps-command-support",
		&common_info->hbm.hbm_fps_command_support, 0);
	if (common_info->hbm.hbm_fps_command_support) {
		lcd_kit_parse_u32(np, "lcd-kit,hbm-level-base",
			&common_info->hbm.hbm_level_base, 0);
		lcd_kit_parse_u32(np, "lcd-kit,hbm-level-bias",
			&common_info->hbm.hbm_level_bias, 0);
		lcd_kit_parse_u32(np, "lcd-kit,hbm-level-skip-low",
			&common_info->hbm.hbm_level_skip_low, 0);
		lcd_kit_parse_u32(np, "lcd-kit,hbm-level-skip-high",
			&common_info->hbm.hbm_level_skip_high, 0);
	}
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
		common_info->dfr_info.hbm_status = HBM_STATUS_IDLE;
		common_info->dfr_info.fps_dfr_status = FPS_DFR_STATUS_IDLE;
	}
	/* aod exit disp on cmds */
	lcd_kit_parse_dcs_cmds(np, "lcd-kit,panel-aod-exit-dis-on-cmds",
		"lcd-kit,panel-aod-exit-dis-on-cmds-state",
		&common_info->aod_exit_dis_on_cmds);
	/* alpm set */
	lcd_kit_parse_u32(np, "lcd-kit,aod-no-need-init",
		&common_info->aod_no_need_init, 0);
}

static void lcd_kit_parse_btb_check(struct device_node *np)
{
	lcd_kit_parse_u32(np, "lcd-kit,lcd-btb-support",
		&common_info->btb_support, 0);

	if (common_info->btb_support) {
		lcd_kit_parse_u32(np, "lcd-kit,lcd-btb-check-type",
			&common_info->btb_check_type, 0);

		lcd_kit_parse_array_data(np, "lcd-kit,lcd-btb-gpio",
			&common_info->lcd_btb_gpio);
	}
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
	/* btb check */
	lcd_kit_parse_btb_check(np);
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

static void lcd_kit_check_wq_handler(struct work_struct *work)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_bl_ops *bl_ops = NULL;
	struct lcd_kit_ops *lcd_ops = NULL;

	lcd_ops = lcd_kit_get_ops();
	bl_ops = lcd_kit_get_bl_ops();
	if (!common_info->check_thread.enable) {
		LCD_KIT_DEBUG("check_thread is disable\n");
		return;
	}
	if (common_info->set_vss.support) {
		if (lcd_ops && lcd_ops->set_vss_by_thermal) {
			ret = lcd_ops->set_vss_by_thermal();
			if (ret)
				LCD_KIT_ERR("set vss by thermal failed!\n");
		}
	}
	if (common_info->set_power.support) {
		if (lcd_ops && lcd_ops->set_power_by_thermal) {
			ret = lcd_ops->set_power_by_thermal();
			if (ret)
				LCD_KIT_ERR("set vss by thermal failed!\n");
		}
	}
	if (common_info->check_thread.check_bl_support) {
		/* check backlight */
		if (bl_ops && bl_ops->check_backlight) {
			ret = bl_ops->check_backlight();
			if (ret)
				LCD_KIT_ERR("backlight check abnomal!\n");
		}
	}
}

static enum hrtimer_restart lcd_kit_check_hrtimer_fnc(struct hrtimer *timer)
{
	if (common_info->check_thread.enable) {
		schedule_delayed_work(&common_info->check_thread.check_work, 0);
		hrtimer_start(&common_info->check_thread.hrtimer,
			ktime_set(CHECK_THREAD_TIME_PERIOD / 1000, // chang ms to s
			(CHECK_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
	}
	return HRTIMER_NORESTART;
}

static void lcd_kit_check_thread_register(void)
{
	if (common_info->check_thread.enable) {
		INIT_DELAYED_WORK(&common_info->check_thread.check_work,
			lcd_kit_check_wq_handler);
		hrtimer_init(&common_info->check_thread.hrtimer,
			CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		common_info->check_thread.hrtimer.function =
			lcd_kit_check_hrtimer_fnc;
		hrtimer_start(&common_info->check_thread.hrtimer,
			ktime_set(CHECK_THREAD_TIME_PERIOD / 1000, // chang ms to s
			(CHECK_THREAD_TIME_PERIOD % 1000) * 1000000), // change ms to ns
			HRTIMER_MODE_REL);
	}
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
}
static int lcd_kit_common_init(struct device_node *np)
{
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node!\n");
		return LCD_KIT_FAIL;
	}

#ifdef LCD_KIT_DEBUG_ENABLE
	lcd_kit_debugfs_init();
#endif
	lcd_kit_panel_parse_dt(np);
	lcd_kit_check_thread_register();
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

/* used to avoid a uint32_t happening overflow */
#define MAX_ERROR_TIMES 100000000
static void lcd_kit_mipi_check(void *pdata, char *panel_name,
	long display_on_record_time)
{
	int i, ret;
	uint32_t read_value[MAX_REG_READ_COUNT] = {0};
	static struct lcd_kit_mipierrors mipi_errors[MAX_REG_READ_COUNT];
	struct lcd_kit_adapt_ops *adapt_ops = lcd_kit_get_adapt_ops();
	uint32_t *expect_ptr = common_info->mipi_check.value.buf;

#if defined CONFIG_HUAWEI_DSM
	#define REC_LIMIT_TIMES (-1)
	#define RECORD_BUFLEN   200
	char record_buf[RECORD_BUFLEN] = {'\0'};
	static int recordtime;
	struct timeval tv = { 0, 0 };
	long diskeeptime = 0;
#endif

	if (!pdata || !panel_name || !adapt_ops || !expect_ptr) {
		LCD_KIT_ERR("mipi check happened parameter error!\n");
		return;
	}
	if (common_info->mipi_check.support == 0)
		return;
	if (!adapt_ops->mipi_rx) {
		LCD_KIT_ERR("mipi_rx function is null!\n");
		return;
	}
	memset(mipi_errors, 0,
		MAX_REG_READ_COUNT * sizeof(struct lcd_kit_mipierrors));
	ret = adapt_ops->mipi_rx(pdata, (u8 *)read_value, MAX_REG_READ_COUNT - 1,
		&common_info->mipi_check.cmds);
	if (ret) {
		LCD_KIT_ERR("mipi read failed!\n");
		return;
	}
	for (i = 0; i < common_info->mipi_check.value.cnt; i++) {
		if (mipi_errors[i].total_errors >= MAX_ERROR_TIMES) {
			LCD_KIT_ERR("mipi error times is too large!\n");
			return;
		}
		mipi_errors[i].mipi_check_times++;
		if (read_value[i] != expect_ptr[i]) {
			mipi_errors[i].mipi_error_times++;
			mipi_errors[i].total_errors += read_value[i];
			LCD_KIT_ERR("mipi check error[%d]: current error times:%d! total error times:%d, check-error-times/check-times:%d/%d\n",
				i, read_value[i], mipi_errors[i].total_errors,
				mipi_errors[i].mipi_error_times,
				mipi_errors[i].mipi_check_times);
#if defined CONFIG_HUAWEI_DSM
			if (read_value[i] < common_info->mipi_check.mipi_error_report_threshold)
				continue;
			do_gettimeofday(&tv);
			diskeeptime = tv.tv_sec - display_on_record_time;
			ret = snprintf(record_buf, RECORD_BUFLEN, "%s:display_on_keep_time=%lds, reg_val[%d]=0x%x!\n",
				"lcd", diskeeptime, common_info->mipi_check.cmds.cmds[i].payload[0], read_value[i]);
			if (ret < 0) {
				LCD_KIT_ERR("snprintf happened error!\n");
				continue;
			}
			(void)lcd_dsm_client_record(lcd_dclient, record_buf,
				DSM_LCD_MIPI_TRANSMIT_ERROR_NO, REC_LIMIT_TIMES, &recordtime);
#endif
			continue;
		}
		LCD_KIT_INFO("mipi check nomal[%d]: total error times:%d, check-error-times/check-times:%d/%d\n",
			i, mipi_errors[i].total_errors, mipi_errors[i].mipi_error_times, mipi_errors[i].mipi_check_times);
	}
}

static void lcd_kit_btb_check(void)
{
	int i;
	int btb_status = 0;
	int btb_gpio;

	if (!common_info->btb_support) {
		LCD_KIT_INFO("LCD BTB check not support\n");
		return;
	}

	for (i = 0; i < common_info->lcd_btb_gpio.cnt; i++) {
		btb_gpio = common_info->lcd_btb_gpio.buf[i] + common_info->gpio_offset;
		btb_status += gpio_get_value(btb_gpio);
	}

	if (btb_status > 0) {
		LCD_KIT_ERR("LCD BTB check ERROR = %d\n", btb_status);
#if defined CONFIG_HUAWEI_DSM
		dsm_client_record(lcd_dclient, "LCD BTB check ERROR = %d",
			btb_status);
		dsm_client_notify(lcd_dclient, DSM_LCD_BTB_CHECK_ERROR_NO);
#endif
	} else {
		LCD_KIT_INFO("LCD BTB check OK\n");
	}
}

static irqreturn_t lcd_kit_btb_check_handler(int irq, void *dev_id)
{
	if (btb_irq_count < MAX_BTB_IRQ_COUNT){
		lcd_kit_btb_check();
	} else {
		LCD_KIT_ERR("LCD BTB check error is 2 times\n");
		disable_irq_nosync(irq);
	}
	btb_irq_count++;
	return IRQ_HANDLED;
}

static void lcd_kit_btb_init(void)
{
	int ret;
	int btb_gpio;
	int irq;

	if (!common_info->btb_support) {
		LCD_KIT_INFO("LCD BTB check not support\n");
		return;
	}

	LCD_KIT_INFO("btb type = %d\n", common_info->btb_check_type);

	if (common_info->btb_check_type == LCD_KIT_BTB_CHECK_IRQ) {
		/* btb0 is up btb check */
		btb_gpio = common_info->lcd_btb_gpio.buf[0] + common_info->gpio_offset;
		ret = gpio_request(btb_gpio, "btb_gpio0");
		if (ret < 0) {
			LCD_KIT_ERR("request gpio[%d] failed\n", btb_gpio);
			return;
		}

		gpio_direction_input(btb_gpio);
		irq = gpio_to_irq(btb_gpio);
		ret = request_irq(irq, lcd_kit_btb_check_handler,
			IRQF_TRIGGER_HIGH, "btb0", NULL);
		if (ret)
			LCD_KIT_ERR("btb request irq0 failed\n");

		/* btb1 is down btb check */
		btb_gpio = common_info->lcd_btb_gpio.buf[1] + common_info->gpio_offset;
		ret = gpio_request(btb_gpio, "btb_gpio1");
		if (ret < 0) {
			LCD_KIT_ERR("request gpio[%d] failed\n", btb_gpio);
			return;
		}

		gpio_direction_input(btb_gpio);
		irq = gpio_to_irq(btb_gpio);
		ret = request_irq(irq, lcd_kit_btb_check_handler,
			IRQF_TRIGGER_HIGH, "btb1", NULL);
		if (ret)
			LCD_KIT_ERR("btb request irq1 failed\n");
	}
}

void lcd_backlight_i2c_dmd(void)
{
	if (lcd_backlight_i2c_count < MAX_I2C_DMD_COUNT) {
#if defined CONFIG_HUAWEI_DSM
		dsm_client_record(lcd_dclient, "lcd backlight i2c error\n");
		dsm_client_notify(lcd_dclient, DSM_LCD_BACKLIGHT_I2C_ERROR_NO);
#endif
		lcd_backlight_i2c_count++;
	}
}
void lcd_bias_i2c_dmd(void)
{
	if (lcd_bias_i2c_count < MAX_I2C_DMD_COUNT) {
#if defined CONFIG_HUAWEI_DSM
		dsm_client_record(lcd_dclient, "lcd bias i2c error\n");
		dsm_client_notify(lcd_dclient, DSM_LCD_BIAS_I2C_ERROR_NO);
#endif
		lcd_bias_i2c_count++;
	}
}

/* common ops */
struct lcd_kit_common_ops g_lcd_kit_common_ops = {
	.common_init = lcd_kit_common_init,
	.panel_power_on = lcd_kit_panel_power_on,
	.panel_power_on_without_mipi = lcd_kit_panel_power_on_without_mipi,
	.panel_power_on_with_mipi = lcd_kit_panel_power_on_with_mipi,
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
	.dsi_handle = lcd_kit_dsi_handle,
	.dirty_region_handle = lcd_kit_dirty_region_handle,
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
	.mipi_check = lcd_kit_mipi_check,
	.btb_check = lcd_kit_btb_check,
	.btb_init = lcd_kit_btb_init,
};

struct gamma_linear_interpolation_info {
	uint32_t grayscale_before;
	uint32_t grayscale;
	uint32_t grayscale_after;
	uint32_t gamma_node_value_before;
	uint32_t gamma_node_value;
	uint32_t gamma_node_value_after;
};

static int get_grayscale_index(int grayscale, const struct gamma_node_info *gamma_info, int *index)
{
	int i;
	int cnt;

	if (!gamma_info || !index) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}
	cnt = gamma_info->node_grayscale.cnt;
	if (grayscale <= gamma_info->node_grayscale.buf[0]) {
		*index = 1;
	} else if (grayscale >= gamma_info->node_grayscale.buf[cnt - 1]) {
		*index = cnt - 1;
	} else {
		for (i = 0; i < cnt; i++) {
			LCD_KIT_DEBUG("grayscale[%d]:%d\n", i, gamma_info->node_grayscale.buf[i]);
			if (grayscale <= gamma_info->node_grayscale.buf[i]) {
				*index = i;
				break;
			}
		}
	}
	return LCD_KIT_OK;
}

static int linear_interpolation_calculation_gamma(struct gamma_linear_interpolation_info *gamma_liner_info)
{
	int gamma_value = -1;

	if (!gamma_liner_info) {
		LCD_KIT_ERR("gamma_liner_info is NULL\n");
		return LCD_KIT_FAIL;
	}

	if ((gamma_liner_info->grayscale_before - gamma_liner_info->grayscale_after) == 0) {
		gamma_liner_info->gamma_node_value = gamma_liner_info->gamma_node_value_before;
	} else {
		/* Multiply by 100 to avoid loss of precision, plus 50 to complete rounding */
		gamma_value = (100 * ((int)gamma_liner_info->grayscale -
			(int)gamma_liner_info->grayscale_after) *
			((int)gamma_liner_info->gamma_node_value_before -
			(int)gamma_liner_info->gamma_node_value_after) /
			((int)gamma_liner_info->grayscale_before -
			(int)gamma_liner_info->grayscale_after)) +
			(100 * ((int)gamma_liner_info->gamma_node_value_after));
		gamma_liner_info->gamma_node_value = (uint32_t)(gamma_value + 50) / 100;
	}
	LCD_KIT_DEBUG("grayscale_before:%d, grayscale:%d, grayscale_after:%d,"
		"gamma_node_value_before:%d, gamma_node_value:%d, gamma_node_value_after:%d, gamma_value:%d\n",
		gamma_liner_info->grayscale_before, gamma_liner_info->grayscale,
		gamma_liner_info->grayscale_after, gamma_liner_info->gamma_node_value_before,
		gamma_liner_info->gamma_node_value, gamma_liner_info->gamma_node_value_after,
		gamma_value);
	return LCD_KIT_OK;
}

static int set_gamma_liner_info(const struct gamma_node_info *gamma_info, const uint32_t *gamma_value_array,
	size_t array_size, struct gamma_linear_interpolation_info *gamma_liner_info, int grayscale)
{
	int index;
	int ret;
	int i;

	if (!gamma_info || !gamma_liner_info || !gamma_value_array) {
		LCD_KIT_ERR("NULL pointer\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < array_size; i++)
		LCD_KIT_DEBUG("gamma_value_array[%d]:%d\n", i, gamma_value_array[i]);
	ret = get_grayscale_index(grayscale, gamma_info, &index);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("get_grayscale_index error\n");
		return LCD_KIT_FAIL;
	}
	if ((((uint32_t)index) >= array_size) || (index >= gamma_info->node_grayscale.cnt)) {
		LCD_KIT_ERR("index out of range, index:%d, node_grayscale.cnt:%d, gamma_value_array's size:%d\n",
			index, gamma_info->node_grayscale.cnt, array_size);
		return LCD_KIT_FAIL;
	}
	gamma_liner_info->grayscale_before = gamma_info->node_grayscale.buf[index - 1];
	gamma_liner_info->grayscale = grayscale;
	gamma_liner_info->grayscale_after = gamma_info->node_grayscale.buf[index];
	gamma_liner_info->gamma_node_value_before = gamma_value_array[index - 1];
	gamma_liner_info->gamma_node_value_after = gamma_value_array[index];
	LCD_KIT_DEBUG("index:%d, gamma_node_value_before:%d, gamma_node_value_after:%d\n",
		index, gamma_liner_info->gamma_node_value_before, gamma_liner_info->gamma_node_value_after);
	return ret;
}

static int set_color_cmds_value_by_grayscale(const struct gamma_node_info *gamma_info,
	const uint32_t *gamma_value_array, size_t array_size, int *payload, int grayscale)
{
	int ret = LCD_KIT_OK;
	struct gamma_linear_interpolation_info gamma_liner_info;

	if (!payload) {
		LCD_KIT_ERR("payload is NULL\n");
		return LCD_KIT_FAIL;
	}
	ret = set_gamma_liner_info(gamma_info, gamma_value_array, array_size, &gamma_liner_info, grayscale);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("set_gamma_liner_info error\n");
		return LCD_KIT_FAIL;
	}
	linear_interpolation_calculation_gamma(&gamma_liner_info);

	/* High 8 bits correspond payload[0], low 8 bits correspond payload[1] */
	payload[0] = (gamma_liner_info.gamma_node_value >> 8) & 0xff;
	payload[1] = gamma_liner_info.gamma_node_value & 0xff;
	LCD_KIT_DEBUG("payload[1]:%d, payload[2]:%d\n", payload[0], payload[1]);
	return ret;
}

static int set_color_cmds_by_grayscale(uint32_t fps, struct color_cmds_rgb *color_cmds,
	const struct gamma_node_info *gamma_info, int grayscale)
{
	LCD_KIT_DEBUG("grayscale = %d\n", grayscale);
	if (!color_cmds) {
		LCD_KIT_ERR("color_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (!gamma_info) {
		LCD_KIT_ERR("gamma_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	if (fps == fps_normal) {
		if (set_color_cmds_value_by_grayscale(gamma_info, gamma_info->red_60_hz,
			HBM_GAMMA_NODE_SIZE, color_cmds->red_payload, grayscale) ||
			set_color_cmds_value_by_grayscale(gamma_info, gamma_info->green_60_hz,
			HBM_GAMMA_NODE_SIZE, color_cmds->green_payload, grayscale) ||
			set_color_cmds_value_by_grayscale(gamma_info, gamma_info->blue_60_hz,
			HBM_GAMMA_NODE_SIZE, color_cmds->blue_payload, grayscale)) {
			LCD_KIT_ERR("set_color_cmds_value error\n");
			return LCD_KIT_FAIL;
		}
	} else if (fps == fps_medium) {
		if (set_color_cmds_value_by_grayscale(gamma_info, gamma_info->red_90_hz,
			HBM_GAMMA_NODE_SIZE, color_cmds->red_payload, grayscale) ||
			set_color_cmds_value_by_grayscale(gamma_info, gamma_info->green_90_hz,
			HBM_GAMMA_NODE_SIZE, color_cmds->green_payload, grayscale) ||
			set_color_cmds_value_by_grayscale(gamma_info, gamma_info->blue_90_hz,
			HBM_GAMMA_NODE_SIZE, color_cmds->blue_payload, grayscale)) {
			LCD_KIT_ERR("set_color_cmds_value error\n");
			return LCD_KIT_FAIL;
		}
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static int set_60hz_color_cmds_value_by_index(int gamma_node_index)
{
	/* The interval between gamma nodes in the array is 4 */
	int start_pos = gamma_node_index * 4;
	if ((start_pos + 1) >= HBM_GAMMA_SIZE) {
		LCD_KIT_ERR("start_pos out of range, start_pos:%d,"
			"HBM_GAMMA_SIZE:%d\n", start_pos, HBM_GAMMA_SIZE);
		return LCD_KIT_FAIL;
	}

	/* 14 is the 15th of the payload */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen <= 14) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}

	/* R, 1 indicates the second command line,
	9 and 10 are the 10th and 11th of the payload */
	common_info->hbm.circle_color_cmds.cmds[1].payload[9] =
		common_info->hbm.hbm_gamma.red_60_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[1].payload[10] =
		common_info->hbm.hbm_gamma.red_60_hz[start_pos + 1];

	/* G, 1 indicates the second command line,
	11 and 12 are the 12th and 13th of the payload */
	common_info->hbm.circle_color_cmds.cmds[1].payload[11] =
		common_info->hbm.hbm_gamma.green_60_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[1].payload[12] =
		common_info->hbm.hbm_gamma.green_60_hz[start_pos + 1];

	/* B, 1 indicates the second command line,
		13 and 14 are the 14th and 15th of the payload */
	common_info->hbm.circle_color_cmds.cmds[1].payload[13] =
		common_info->hbm.hbm_gamma.blue_60_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[1].payload[14] =
		common_info->hbm.hbm_gamma.blue_60_hz[start_pos + 1];

	return LCD_KIT_OK;
}

static int set_90hz_color_cmds_value_by_index(int gamma_node_index)
{
	/* The interval between gamma nodes in the array is 4 */
	int start_pos = gamma_node_index * 4;
	if ((start_pos + 1) >= HBM_GAMMA_SIZE) {
		LCD_KIT_ERR("start_pos out of range, start_pos:%d,"
		"HBM_GAMMA_SIZE:%d\n", start_pos, HBM_GAMMA_SIZE);
		return LCD_KIT_FAIL;
	}

	/* 14 is the 15th of the payload */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen <= 14) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}

	/* R, 1 indicates the second command line,
	9 and 10 are the 10th and 11th of the payload */
	common_info->hbm.circle_color_cmds.cmds[1].payload[9] =
		common_info->hbm.hbm_gamma.red_90_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[1].payload[10] =
		common_info->hbm.hbm_gamma.red_90_hz[start_pos + 1];

	/* G, 1 indicates the second command line,
	11 and 12 are the 12th and 13th of the payload */
	common_info->hbm.circle_color_cmds.cmds[1].payload[11] =
		common_info->hbm.hbm_gamma.green_90_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[1].payload[12] =
		common_info->hbm.hbm_gamma.green_90_hz[start_pos + 1];

	/* B, 1 indicates the second command line,
	13 and 14 are the 14th and 15th of the payload */
	common_info->hbm.circle_color_cmds.cmds[1].payload[13] =
		common_info->hbm.hbm_gamma.blue_90_hz[start_pos];
	common_info->hbm.circle_color_cmds.cmds[1].payload[14] =
		common_info->hbm.hbm_gamma.blue_90_hz[start_pos + 1];

	return LCD_KIT_OK;
}

int lcd_kit_local_hbm_set_color_cmds_value_by_index(const uint32_t fps, int gamma_node_index)
{
	int ret;
	if (common_info->hbm.circle_color_cmds.cmds == NULL) {
		LCD_KIT_ERR("circle_color_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("gamma_node_index:%d, fps:%d\n", gamma_node_index, fps);
	if (fps == fps_normal) {
		ret = set_60hz_color_cmds_value_by_index(gamma_node_index);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_DEBUG("set_60hz_color_cmds_value_by_index error\n");
			return LCD_KIT_FAIL;
		}
	} else if (fps == fps_medium) {
		ret = set_90hz_color_cmds_value_by_index(gamma_node_index);
		if (ret != LCD_KIT_OK) {
			LCD_KIT_DEBUG("set_90hz_color_cmds_value_by_index error\n");
			return LCD_KIT_FAIL;
		}
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	return ret;
}

static int set_gamma_node_info(struct gamma_node_info *gamma_info)
{
	int i;
	int cnt;

	if (!gamma_info) {
		LCD_KIT_ERR("gamma_info is NULL\n");
		return LCD_KIT_FAIL;
	}
	cnt = gamma_info->node_grayscale.cnt;

	/* 2 hbm_gamma_values are converted to 1 gamma_node_value. */
	if ((cnt > HBM_GAMMA_NODE_SIZE) || ((2 * cnt) > HBM_GAMMA_SIZE)) {
		LCD_KIT_ERR("node_grayscale.cnt out of range,"
			"gamma_info->node_grayscale.cnt:%d, HBM_GAMMA_NODE_SIZE:%d,"
			"HBM_GAMMA_SIZE:%d\n", cnt, HBM_GAMMA_NODE_SIZE, HBM_GAMMA_SIZE);
		return LCD_KIT_FAIL;
	}

	/* The high 8 bits of a hex number are converted into a dec number multiplied by 256 */
	for (i = 0; i < cnt; i++) {
		gamma_info->red_60_hz[i] = (common_info->hbm.hbm_gamma.red_60_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.red_60_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->red_60_hz[%d]:%d\n", i, gamma_info->red_60_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->green_60_hz[i] = (common_info->hbm.hbm_gamma.green_60_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.green_60_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->green_60_hz[%d]:%d\n", i, gamma_info->green_60_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->blue_60_hz[i] = (common_info->hbm.hbm_gamma.blue_60_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.blue_60_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->blue_60_hz[%d]:%d\n", i, gamma_info->blue_60_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->red_90_hz[i] = (common_info->hbm.hbm_gamma.red_90_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.red_90_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->red_90_hz[%d]:%d\n", i, gamma_info->red_90_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->green_90_hz[i] = (common_info->hbm.hbm_gamma.green_90_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.green_90_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->green_90_hz[%d]:%d\n", i, gamma_info->green_90_hz[i]);
	}
	for (i = 0; i < cnt; i++) {
		gamma_info->blue_90_hz[i] = (common_info->hbm.hbm_gamma.blue_90_hz[i * 2] * 256) +
			common_info->hbm.hbm_gamma.blue_90_hz[(i * 2) + 1];
		LCD_KIT_INFO("gamma_info->blue_90_hz[%d]:%d\n", i, gamma_info->blue_90_hz[i]);
	}
	return LCD_KIT_OK;
}

static int set_hbm_gamma_60hz_to_common_info(uint8_t *hbm_gamma, size_t rgb_size)
{
	int i;

	if (!hbm_gamma) {
		LCD_KIT_ERR("hbm_gamma is NULL\n");
		return LCD_KIT_FAIL;
	}

	if ((GAMMA_INDEX_BLUE_HIGH + HBM_GAMMA_SIZE) > rgb_size) {
		LCD_KIT_ERR("hbm_gamma hbm_gamma index out of range\n");
		return LCD_KIT_FAIL;
	}

	/* The 2 values correspond to each other */
	for (i = 0; i < (HBM_GAMMA_SIZE / 2); i++) {
		common_info->hbm.hbm_gamma.red_60_hz[i * 2] = hbm_gamma[GAMMA_INDEX_RED_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.red_60_hz[(i * 2) + 1] = hbm_gamma[GAMMA_INDEX_RED_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.green_60_hz[i * 2] = hbm_gamma[GAMMA_INDEX_GREEN_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.green_60_hz[(i * 2) + 1] = hbm_gamma[GAMMA_INDEX_GREEN_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.blue_60_hz[i * 2] = hbm_gamma[GAMMA_INDEX_BLUE_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.blue_60_hz[(i * 2) + 1] = hbm_gamma[GAMMA_INDEX_BLUE_LOW + (i * 2)];
	}

	for (i = 0; i < rgb_size; i++)
		LCD_KIT_INFO("hbm 60 gamma %d = 0x%02X\n", i, hbm_gamma[i]);
	return LCD_KIT_OK;
}

static int set_hbm_gamma_90hz_to_common_info(uint8_t *hbm_gamma, size_t rgb_size)
{
	int i;

	if (!hbm_gamma) {
		LCD_KIT_ERR("hbm_gamma is NULL\n");
		return LCD_KIT_FAIL;
	}
	if ((GAMMA_INDEX_BLUE_HIGH + HBM_GAMMA_SIZE) > rgb_size) {
		LCD_KIT_ERR("hbm_gamma index out of range\n");
		return LCD_KIT_FAIL;
	}

	// The 2 values correspond to each other
	for (i = 0; i < (HBM_GAMMA_SIZE / 2); i++) {
		common_info->hbm.hbm_gamma.red_90_hz[i * 2] = hbm_gamma[GAMMA_INDEX_RED_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.red_90_hz[(i * 2) + 1] = hbm_gamma[GAMMA_INDEX_RED_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.green_90_hz[i * 2] = hbm_gamma[GAMMA_INDEX_GREEN_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.green_90_hz[(i * 2) + 1] = hbm_gamma[GAMMA_INDEX_GREEN_LOW + (i * 2)];
		common_info->hbm.hbm_gamma.blue_90_hz[i * 2] = hbm_gamma[GAMMA_INDEX_BLUE_HIGH + (i * 2)];
		common_info->hbm.hbm_gamma.blue_90_hz[(i * 2) + 1] = hbm_gamma[GAMMA_INDEX_BLUE_LOW + (i * 2)];
	}

	for (i = 0; i < rgb_size; i++)
		LCD_KIT_INFO("hbm 90 gamma %d = 0x%02X\n", i, hbm_gamma[i]);
	return LCD_KIT_OK;
}

void lcd_kit_hbm_gamma_read(void *hld)
{
	/* read rgb gamma, hbm_gamma structure: */
	/* [red_high, red_low, green_high, green_low, blue_high, blue_low] */
	/* read 60 hz gamma, number 3 corresponds to RGB */
	const size_t rgb_size = HBM_GAMMA_SIZE * 3;
	uint8_t hbm_gamma[rgb_size];
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!common_info->hbm.local_hbm_support)
		LCD_KIT_ERR("local_hbm not support\n");

	LCD_KIT_INFO("lcd_kit_hbm_gamma_read\n");
	/* init check_flag */
	common_info->hbm.hbm_gamma.check_flag = 0;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return;
	}

	if (adapt_ops->mipi_rx(hld, hbm_gamma, rgb_size - 1, &common_info->hbm.hbm_60_hz_gamma_read_cmds)) {
		LCD_KIT_ERR("hbm 60 HZ gamma mipi_rx failed!\n");
		return;
	}
	if (set_hbm_gamma_60hz_to_common_info(hbm_gamma, rgb_size)) {
		LCD_KIT_ERR("set_hbm_gamma_60hz_to_common_info error\n");
		return;
	}

	// read 90 hz gamma
	if (adapt_ops->mipi_rx(hld, hbm_gamma, rgb_size - 1, &common_info->hbm.hbm_90_hz_gamma_read_cmds)) {
		LCD_KIT_ERR("hbm 90 HZ gamma mipi_rx failed!\n");
		return;
	}
	if (set_hbm_gamma_90hz_to_common_info(hbm_gamma, rgb_size)) {
		LCD_KIT_ERR("set_hbm_gamma_90hz_to_common_info error\n");
		return;
	}
	set_gamma_node_info(&common_info->hbm.gamma_info);
}

static int lcd_kit_gamma_code_print(void)
{
	/* 9、10 is payload of R, 11、12 is payload of G,
	13、14 is payload of B */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen <= 14) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("DDDDcircle_color_cmds.cmds[1].payload[9]=%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].payload[9]);
	LCD_KIT_DEBUG("DDDDcircle_color_cmds.cmds[1].payload[10]=%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].payload[10]);
	LCD_KIT_DEBUG("DDDDcircle_color_cmds.cmds[1].payload[11]=%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].payload[11]);
	LCD_KIT_DEBUG("DDDDcircle_color_cmds.cmds[1].payload[12]=%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].payload[12]);
	LCD_KIT_DEBUG("DDDDcircle_color_cmds.cmds[1].payload[13]=%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].payload[13]);
	LCD_KIT_DEBUG("DDDDcircle_color_cmds.cmds[1].payload[14]=%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].payload[14]);
	return LCD_KIT_OK;
}

int lcd_kit_local_hbm_set_color_by_index(void *hld, uint32_t fps,
	uint32_t gamma_node_index)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!common_info->hbm.local_hbm_support) {
		LCD_KIT_DEBUG("local_hbm not support\n");
		return LCD_KIT_OK;
	}
	if (common_info->hbm.circle_color_cmds.cmds == NULL ||
		common_info->hbm.hbm_em_configure_60hz_cmds.cmds == NULL ||
		common_info->hbm.hbm_em_configure_90hz_cmds.cmds == NULL) {
		LCD_KIT_ERR("cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_local_hbm_set_color_cmds_value_by_index(fps,
		gamma_node_index);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_local_hbm_set_color_cmds_value_by_index"
			" error\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_rx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_gamma_code_print();
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_gamma_code_print error\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(hld, &(common_info->hbm.circle_color_cmds));
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx faild! ret = %d\n", ret);
	return ret;
}

static int lcd_kit_copy_color_cmds_to_common_info(
	struct color_cmds_rgb *color_cmds)
{
	if (!color_cmds) {
		LCD_KIT_ERR("color_cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	/* 14 is the 15th of the payload */
	if ((int)common_info->hbm.circle_color_cmds.cmds[1].dlen <= 14) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.circle_color_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}

	/* 9、10 is payload of R, 11、12 is payload of G,
	13、14 is payload of B */
	common_info->hbm.circle_color_cmds.cmds[1].payload[9] =
		color_cmds->red_payload[0];
	common_info->hbm.circle_color_cmds.cmds[1].payload[10] =
		color_cmds->red_payload[1];
	common_info->hbm.circle_color_cmds.cmds[1].payload[11] =
		color_cmds->green_payload[0];
	common_info->hbm.circle_color_cmds.cmds[1].payload[12] =
		color_cmds->green_payload[1];
	common_info->hbm.circle_color_cmds.cmds[1].payload[13] =
		color_cmds->blue_payload[0];
	common_info->hbm.circle_color_cmds.cmds[1].payload[14] =
		color_cmds->blue_payload[1];

	return LCD_KIT_OK;
}

int lcd_kit_local_hbm_set_color_by_grayscale(void *hld, uint32_t fps,
	int grayscale)
{
	int ret;
	struct color_cmds_rgb color_cmds;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (common_info->hbm.circle_color_cmds.cmds == NULL) {
		LCD_KIT_ERR("circle_color_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	LCD_KIT_INFO("grayscale = %d\n", grayscale);
	ret = set_color_cmds_by_grayscale(fps, &color_cmds,
		&common_info->hbm.gamma_info, grayscale);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("set_color_cmds error\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_copy_color_cmds_to_common_info(&color_cmds);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_copy_color_cmds_to_common_info error\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_gamma_code_print();
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_gamma_code_print error\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	ret = adapt_ops->mipi_tx(hld, &(common_info->hbm.circle_color_cmds));
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx faild! ret = %d\n", ret);
	return ret;
}

static int lcd_kit_set_em_configure(void *hld, uint32_t fps)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (fps == fps_normal) {
		ret = adapt_ops->mipi_tx(hld,
			&(common_info->hbm.hbm_em_configure_60hz_cmds));
	} else if (fps == fps_medium) {
		ret = adapt_ops->mipi_tx(hld,
			&(common_info->hbm.hbm_em_configure_90hz_cmds));
	} else {
		LCD_KIT_ERR("fps not support, fps:%d\n", fps);
		return LCD_KIT_FAIL;
	}
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx failed! ret = %d\n", ret);

	return ret;
}

int lcd_kit_local_hbm_set_dbv(void *hld, uint32_t dbv, uint32_t fps)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!hld) {
		LCD_KIT_ERR("hld is NULL\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("set dbv, dbv value:%d\n", dbv);
	if (common_info->hbm.hbm_dbv_cmds.cmds == NULL) {
		LCD_KIT_ERR("hbm_dbv_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_set_em_configure(hld, fps);
	if (ret != LCD_KIT_OK) {
		LCD_KIT_ERR("lcd_kit_set_em_configure error\n");
		return LCD_KIT_FAIL;
	}

	/* hbm_dbv_cmds.cmd_cnt's maximum value is 2 */
	if (common_info->hbm.hbm_dbv_cmds.cmd_cnt != 2) {
		LCD_KIT_ERR("hbm_dbv_cmds.cmd_cnt does not match\n");
		return LCD_KIT_FAIL;
	}

	/* 25 is the 16th of the payload of dbv */
	if ((int)common_info->hbm.hbm_dbv_cmds.cmds[1].dlen <= 25) {
		LCD_KIT_ERR("array index out of range, cmds[1].dlen:%d\n",
			common_info->hbm.hbm_dbv_cmds.cmds[1].dlen);
		return LCD_KIT_FAIL;
	}

	/* 1 indicates the command in the second line,
	24 and 25 are dbv payloads */
	common_info->hbm.hbm_dbv_cmds.cmds[1].payload[24] = (dbv >> 8) & 0x0f;
	common_info->hbm.hbm_dbv_cmds.cmds[1].payload[25] = dbv & 0xff;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(hld, &common_info->hbm.hbm_dbv_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("mipi_tx faild! ret = %d\n", ret);
	return ret;
}

int lcd_kit_local_hbm_set_circle(void *hld, bool is_on)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (!hld) {
		LCD_KIT_ERR("hld is null\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	if (!is_on) {
		LCD_KIT_DEBUG("circle exit\n");
		if (common_info->hbm.exit_circle_cmds.cmds == NULL) {
			LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
			return LCD_KIT_FAIL;
		}
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.exit_circle_cmds);
	} else {
		if (common_info->hbm.enter_circle_cmds.cmds == NULL) {
			LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
			return LCD_KIT_FAIL;
		}
		LCD_KIT_DEBUG("circle enter\n");
		ret = adapt_ops->mipi_tx(hld, &common_info->hbm.enter_circle_cmds);
	}
	return ret;
}

u32 lcd_kit_get_mipi_level(void)
{
#ifdef LV_GET_LCDBK_ON
	return mipi_level;
#else
	return 0;
#endif
}

int lcd_kit_enter_ddic_alpha(void *hld, uint32_t alpha)
{
	int ret;
	int payload_num;
	int cmd_cnt;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (common_info->ddic_alpha.enter_alpha_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	cmd_cnt = common_info->ddic_alpha.enter_alpha_cmds.cmd_cnt;
	if (cmd_cnt <= 0 || cmd_cnt > cmd_cnt_max) {
		LCD_KIT_ERR("enter_alpha_cmds's cmd_cnt is invalid, cmd_cnt:%d\n", cmd_cnt);
		return LCD_KIT_FAIL;
	}
	payload_num = (int)common_info->ddic_alpha.enter_alpha_cmds.cmds[cmd_cnt - 1].dlen;
	if (payload_num < payload_num_min || payload_num > payload_num_max) {
		LCD_KIT_ERR("enter_alpha_cmds.cmds's payload_num is invalid, payload_num:%d\n", payload_num);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("cmd_cnt = %d, payload_num = %d\n", cmd_cnt, payload_num);

	/* Sets the upper eight bits of the alpha. */
	if (common_info->ddic_alpha.alpha_with_enable_flag == 0)
		common_info->ddic_alpha.enter_alpha_cmds.cmds[cmd_cnt - 1].payload[payload_num - 2] =
			(alpha >> 8) & 0x0f;
	else
		common_info->ddic_alpha.enter_alpha_cmds.cmds[cmd_cnt - 1].payload[payload_num - 2] =
			((alpha >> 8) & 0x0f) | 0x10;

	/* Sets the lower eight bits of the alpha. */
	common_info->ddic_alpha.enter_alpha_cmds.cmds[cmd_cnt - 1].payload[payload_num - 1] =
		alpha & 0xff;
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(hld, &common_info->ddic_alpha.enter_alpha_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("adapt_ops->mipi_tx error\n");

	return ret;
}

int lcd_kit_exit_ddic_alpha(void *hld, int alpha)
{
	int ret;
	struct lcd_kit_adapt_ops *adapt_ops = NULL;

	if (common_info->ddic_alpha.exit_alpha_cmds.cmds == NULL) {
		LCD_KIT_ERR("enter_alpha_cmds.cmds is NULL\n");
		return LCD_KIT_FAIL;
	}
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops || !adapt_ops->mipi_tx) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}
	ret = adapt_ops->mipi_tx(hld, &common_info->ddic_alpha.exit_alpha_cmds);
	if (ret != LCD_KIT_OK)
		LCD_KIT_ERR("adapt_ops->mipi_tx error\n");
	return ret;
}

u32 lcd_kit_get_alpha_support(void)
{
	return common_info->ddic_alpha.alpha_support;
}

u32 lcd_kit_get_force_delta_bl_update_support(void)
{
	return common_info->force_delta_bl_update_support;
}

u32 lcd_kit_get_local_hbm_support(void)
{
	return common_info->hbm.local_hbm_support;
}

int lcd_kit_local_hbm_alpha_circle_with_alpha_and_dbv(void *hld, uint32_t fps,
	bool is_on, uint32_t alpha, uint32_t dbv)
{
	LCD_KIT_DEBUG("local hbm mipi_level src:%d\n", dbv);
	if (!hld) {
		LCD_KIT_ERR("hld is null\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_DEBUG("alpha = %d\n", alpha);
	if (!is_on) {
		LCD_KIT_DEBUG("alpha circle exit\n");
		lcd_kit_exit_ddic_alpha(hld, LCD_KIT_ALPHA_DEFAULT);
		lcd_kit_local_hbm_set_circle(hld, false);
	} else {
		if (alpha > LCD_KIT_ALPHA_DEFAULT)
			alpha = LCD_KIT_ALPHA_DEFAULT;

		LCD_KIT_DEBUG("alpha circle enter\n");
		lcd_kit_local_hbm_set_dbv(hld, dbv, fps);
		lcd_kit_enter_ddic_alpha(hld, alpha);
		lcd_kit_local_hbm_set_circle(hld, true);
	}
	return LCD_KIT_OK;
}
