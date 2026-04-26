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
#ifndef CONFIG_DRM_MEDIATEK
#define LOG_TAG "LCM"

#include "lcm_drv.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_common.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_core.h"
#include "bias_bl_utils.h"
#include "ddp_drv.h"
#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else

#ifdef CONFIG_MTK_LEGACY
#include <mach/mt_gpio.h>
#endif
#endif
#ifdef CONFIG_MTK_LEGACY
#include <cust_gpio_usage.h>
#endif
#ifndef CONFIG_FPGA_EARLY_PORTING
#if defined(CONFIG_MTK_LEGACY)
#include <cust_i2c.h>
#endif
#endif

#include "lcm_drv.h"

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

#if defined CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_lcd = {
	.name = "dsm_lcd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

struct dsm_client *lcd_dclient;
struct dsm_lcd_record lcd_record;
#endif

#ifdef CONFIG_MACH_MT6768
// HBM
#define BACKLIGHT_HIGH_LEVEL 1
#define BACKLIGHT_LOW_LEVEL  2
// UD PrintFinger HBM
#define LCD_KIT_FP_HBM_ENTER 1
#define LCD_KIT_FP_HBM_EXIT  2
#define LCD_KIT_ENABLE_ELVSSDIM  0
#define LCD_KIT_DISABLE_ELVSSDIM 1
#define LCD_KIT_ELVSSDIM_NO_WAIT 0

#define HBM_EN_TE_TIMES 0
#define HBM_DIS_TE_TIMES 0
#define TE_60_TIME 16667
#define TE_90_TIME 11111
static struct timer_list g_backlight_timer;
static const unsigned int g_fps_90hz = 90;
#define CMD_HBM_ENABLE		0xE0
#define CMD_HBM_DISABLE		0x20
static bool hbm_en;
static bool hbm_wait;
#endif
static void *g_handle;

struct LCM_UTIL_FUNCS lcm_util_mtk;

static struct mtk_panel_info lcd_kit_pinfo = {0};

static struct lcd_kit_disp_info g_lcd_kit_disp_info;
struct lcd_kit_disp_info *lcd_kit_get_disp_info(void)
{
	return &g_lcd_kit_disp_info;
}

#if defined CONFIG_HUAWEI_DSM
struct dsm_client *lcd_kit_get_lcd_dsm_client(void)
{
	return lcd_dclient;
}
#endif

int is_mipi_cmd_panel(void)
{
	if (lcd_kit_pinfo.panel_dsi_mode == 0)
		return 1;
	return 0;
}

#ifdef CONFIG_MACH_MT6768
static void lcd_kit_timerout_function(unsigned long arg)
{
	if (disp_info->bl_is_shield_backlight)
		disp_info->bl_is_shield_backlight = false;
	del_timer(&g_backlight_timer);
	disp_info->bl_is_start_timer = false;
	LCD_KIT_INFO("Sheild backlight 1.2s timeout, remove the bl sheild\n");
}

static void lcd_kit_enable_hbm_timer(void)
{
	if (disp_info->bl_is_start_timer == true) {
		// if timer is not timeout, restart timer
		mod_timer(&g_backlight_timer, (jiffies + 12 * HZ / 10)); // 1.2s
		return;
	}
	init_timer(&g_backlight_timer);
	g_backlight_timer.expires = jiffies + 12 * HZ / 10; // 1.2s
	g_backlight_timer.data = 0;
	g_backlight_timer.function = lcd_kit_timerout_function;
	add_timer(&g_backlight_timer);
	disp_info->bl_is_start_timer = true;
}

static void lcd_kit_disable_hbm_timer(void)
{
	if (disp_info->bl_is_start_timer == false)
		return;

	del_timer(&g_backlight_timer);
	disp_info->bl_is_start_timer = false;
}
#endif

unsigned int esd_recovery_level = 1024;
void  lcd_kit_bl_ic_set_backlight(unsigned int bl_level)
{
	struct lcd_kit_bl_ops *bl_ops = NULL;
	esd_recovery_level = bl_level;

	if (lcd_kit_pinfo.panel_state == LCD_POWER_STATE_OFF) {
		LCD_KIT_ERR("panel is disable!\n");
		return;
	}

	if (lcd_kit_pinfo.bl_ic_ctrl_mode) {
		/* quickly sleep */
		if (disp_info->quickly_sleep_out.support) {
			if (disp_info->quickly_sleep_out.panel_on_tag)
				lcd_kit_disp_on_check_delay();
		}
		bl_ops = lcd_kit_get_bl_ops();
		if (!bl_ops) {
			LCD_KIT_INFO("bl_ops is null!\n");
			return;
		}
		if (bl_ops->set_backlight)
			bl_ops->set_backlight(bl_level);
	}
}

void lcm_set_panel_state(unsigned int state)
{
	lcd_kit_pinfo.panel_state = state;
}

unsigned int lcm_get_panel_state(void)
{
	return lcd_kit_pinfo.panel_state;
}

unsigned int lcm_get_panel_backlight_max_level(void)
{
	return lcd_kit_pinfo.bl_max;
}

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util_mtk, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_esd_config(struct LCM_PARAMS *params)
{
	int i;
	int j;
	int k = 0;
	struct lcd_kit_dsi_cmd_desc *esd_cmds = NULL;

	if (common_info->esd.te_check_support)
		params->dsi.customization_esd_check_enable = ESD_TE_CHECK_ENABLE;
	else
		params->dsi.customization_esd_check_enable = common_info->esd.support;
	params->dsi.esd_check_enable = common_info->esd.support;
	params->dsi.esd_recovery_bl_enable = common_info->esd.recovery_bl_support;
	if (common_info->esd.cmds.cmds) {
		esd_cmds = common_info->esd.cmds.cmds;
		for (i = 0; i < common_info->esd.cmds.cmd_cnt; i++) {
			params->dsi.lcm_esd_check_table[i].cmd = esd_cmds->payload[0];
			params->dsi.lcm_esd_check_table[i].count = esd_cmds->dlen;
			LCD_KIT_INFO("dsi.lcm_esd_check_table[%d].cmd = 0x%x\n",
				i, params->dsi.lcm_esd_check_table[i].cmd);
			LCD_KIT_INFO("dsi.lcm_esd_check_table[%d].count = %d\n",
				i, params->dsi.lcm_esd_check_table[i].count);
			for (j = 0; j < esd_cmds->dlen; j++) {
				params->dsi.lcm_esd_check_table[i].para_list[j] =
					common_info->esd.value.buf[k];
				LCD_KIT_INFO("dsi.lcm_esd_check_table[%d].para_list[%d] = %d\n",
					i, j, params->dsi.lcm_esd_check_table[i].para_list[j]);
				k++;
			}
			esd_cmds++;
		}
	} else {
		LCD_KIT_INFO("esd not config, use default\n");
		params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
		params->dsi.lcm_esd_check_table[0].count = 1;
		params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
	}
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	struct mtk_panel_info *pinfo = &lcd_kit_pinfo;

	memset(params, 0, sizeof(struct LCM_PARAMS));
	LCD_KIT_INFO(" +!\n");
	params->type = pinfo->panel_lcm_type;
	params->width = pinfo->xres;
	params->height = pinfo->yres;
	params->physical_width = pinfo->width;
	params->physical_height = pinfo->height;
	params->physical_width_um = pinfo->width * 1000; // change mm to um
	params->physical_height_um = pinfo->height * 1000; // change mm to um
	params->dsi.mode = pinfo->panel_dsi_mode;
	params->dsi.switch_mode = pinfo->panel_dsi_switch_mode;
	params->dsi.switch_mode_enable = 0;
	params->density = pinfo->panel_density;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = pinfo->mipi.lane_nums;
	/* The following defined the fomat for data coming from LCD engine */
	params->dsi.data_format.color_order = pinfo->bgr_fmt;
	params->dsi.data_format.trans_seq = pinfo->panel_trans_seq;
	params->dsi.data_format.padding = pinfo->panel_data_padding;
	params->dsi.data_format.format = pinfo->bpp;

	/* Highly depends on LCD driver capability */
	params->dsi.packet_size = pinfo->panel_packtet_size;
	/* video mode timing */
	params->dsi.PS = pinfo->panel_ps;
	params->dsi.vertical_sync_active = pinfo->ldi.v_pulse_width;
	params->dsi.vertical_backporch = pinfo->ldi.v_back_porch;
	params->dsi.vertical_frontporch = pinfo->ldi.v_front_porch;
	params->dsi.vertical_frontporch_for_low_power = pinfo->ldi.v_front_porch_forlp;
	params->dsi.vertical_active_line = pinfo->yres;

	params->dsi.horizontal_sync_active = pinfo->ldi.h_pulse_width;
	params->dsi.horizontal_backporch = pinfo->ldi.h_back_porch;
	params->dsi.horizontal_frontporch = pinfo->ldi.h_front_porch;
	params->dsi.horizontal_active_pixel = pinfo->xres;

	/* this value must be in MTK suggested table */
	params->dsi.PLL_CLOCK = pinfo->pxl_clk_rate;
	params->dsi.data_rate = pinfo->data_rate;
	params->dsi.fbk_div =  pinfo->pxl_fbk_div;
	params->dsi.CLK_HS_POST = pinfo->mipi.clk_post_adjust;
	params->dsi.ssc_disable = pinfo->ssc_disable;
	params->dsi.ssc_range = pinfo->ssc_range;
	params->dsi.clk_lp_per_line_enable = pinfo->mipi.lp11_flag;
	params->dsi.rg_vrefsel_vcm_adjust = pinfo->mipi.rg_vrefsel_vcm_adjust;
	if (pinfo->mipi_hopping.switch_en != 0) {
		params->dsi.horizontal_backporch_dyn =
			pinfo->mipi_hopping.h_back_porch;
		params->dsi.PLL_CLOCK_dyn = pinfo->mipi_hopping.data_rate / 2;
		params->dsi.data_rate_dyn = pinfo->mipi_hopping.data_rate;
	}
	/* esd config */
	lcm_get_esd_config(params);
	if (pinfo->mipi.non_continue_en == 0)
		params->dsi.cont_clock = 1;
	else
		params->dsi.cont_clock = 0;
#ifdef CONFIG_MACH_MT6768
	params->hbm_en_time = HBM_EN_TE_TIMES;
	params->hbm_dis_time = HBM_DIS_TE_TIMES;
#endif
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	if (pinfo->round_corner.round_corner_en) {
		params->round_corner_en= pinfo->round_corner.round_corner_en;
		params->full_content = pinfo->round_corner.full_content;
		params->corner_pattern_height = pinfo->round_corner.h;
		params->corner_pattern_height_bot = pinfo->round_corner.h_bot;
		params->corner_pattern_tp_size = pinfo->round_corner.tp_size;
		params->corner_pattern_lt_addr = pinfo->round_corner.lt_addr;
		LCD_KIT_INFO("corner_pattern_tp_size is %u\n", params->corner_pattern_tp_size);
	}
#endif
}

static void lcd_kit_set_thp_proximity_state(int power_state)
{
	if (!common_info->thp_proximity.support) {
		LCD_KIT_INFO("thp_proximity not support!\n");
		return;
	}
	common_info->thp_proximity.panel_power_state = power_state;
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

static void lcd_kit_on(void)
{
	LCD_KIT_INFO(" +!\n");
	lcd_kit_set_thp_proximity_sem(true);
	if (common_ops->panel_power_on)
		common_ops->panel_power_on((void *)NULL);
	lcm_set_panel_state(LCD_POWER_STATE_ON);
	lcd_kit_set_thp_proximity_state(POWER_ON);
	lcd_kit_set_thp_proximity_sem(false);
	/* record panel on time */
	if (disp_info->quickly_sleep_out.support)
		lcd_kit_disp_on_record_time();
	/* btb check */
	if (common_ops->btb_check &&
		common_info->btb_check_type == LCD_KIT_BTB_CHECK_GPIO)
		common_ops->btb_check();
	LCD_KIT_INFO(" -!\n");
}

static void lcd_kit_off(void)
{
	LCD_KIT_INFO(" +!\n");
	if (common_info->hbm.support)
		common_info->hbm.hbm_level_current = 0;

	if (disp_info->bl_is_shield_backlight == true)
		disp_info->bl_is_shield_backlight = false;
#ifdef CONFIG_MACH_MT6768
	lcd_kit_disable_hbm_timer();
#endif
	lcd_kit_set_thp_proximity_sem(true);
	if (common_ops->panel_power_off)
		common_ops->panel_power_off(NULL);
	lcm_set_panel_state(LCD_POWER_STATE_OFF);
	disp_info->alpm_state = ALPM_OUT;
#ifdef CONFIG_MACH_MT6768
	hbm_en = false;
#endif
	lcd_kit_set_thp_proximity_state(POWER_OFF);
	lcd_kit_set_thp_proximity_sem(false);
	LCD_KIT_INFO(" -!\n");
}

static void lcm_resume(void)
{
	lcd_kit_on();
}

static void lcd_kit_set_backlight(void *handle, unsigned int level)
{
	int ret;

	if (disp_info->alpm.support && level)
		lcd_kit_pinfo.bl_current = level;
	if (disp_info->bl_is_shield_backlight)
		return;
	if (level != 0)
		esd_recovery_level = level;
	if (g_handle == NULL)
		g_handle = &ret;

	LCD_KIT_INFO("backlight: level = %u\n", level);

	ret = common_ops->set_mipi_backlight(NULL, level);
	lcd_kit_pinfo.bl_current = level;
	if (disp_info->hbm_entry_delay > 0)
		disp_info->hbm_blcode_ts = ktime_get();
	if (ret < 0)
		return;
}

void lcd_kit_set_backlight_esd_recovery(unsigned int level)
{
	LCD_KIT_INFO("backlight: level recovery level = %u\n", level);
	lcd_kit_set_backlight(NULL, level);
}

#ifdef CONFIG_MACH_MT6768
static void lcm_set_aod_area(void *handle, unsigned char *area)
{
	LCD_KIT_INFO("lcm_set_aod_area +\n");
	return;
}

static void lcm_aod(int enter)
{
	LCD_KIT_INFO("lcm_aod +\n");
	if (enter) {
		LCD_KIT_INFO("lcm_in_setting +\n");
		disp_info->bl_is_shield_backlight = true;
		if (disp_info->alpm.need_reset) {
			/* reset pull low */
			lcd_kit_reset_power_ctrl(0);
			mdelay(1);
			/* reset pull high */
			lcd_kit_reset_power_ctrl(1);
			mdelay(6);
		}
		lcd_kit_aod_cmd_tx(&disp_info->alpm.off_cmds);
		disp_info->alpm_state = ALPM_START;
		disp_info->alpm_start_time = ktime_get();
	} else {
		LCD_KIT_INFO("lcm_out_setting +\n");
		if (disp_info->alpm_state == ALPM_START)
			lcd_kit_aod_cmd_tx(
				&disp_info->alpm.high_light_cmds);
		lcd_kit_aod_cmd_tx(&disp_info->alpm.exit_cmds);
		disp_info->alpm_state = ALPM_OUT;
		disp_info->bl_is_shield_backlight = false;
	}
	LCD_KIT_INFO("lcm_aod -\n");
}

static int lcm_get_doze_delay(void)
{
	return disp_info->alpm.doze_delay;
}

static unsigned long get_real_te_interval(void)
{
	if (disp_info->fps.current_fps == g_fps_90hz) {
		LCD_KIT_INFO("current_fps: 90Hz\n");
		return (unsigned long)TE_90_TIME;
	}

	LCD_KIT_INFO("current_fps: 60Hz\n");
	return (unsigned long)TE_60_TIME;
}

static void lcd_kit_fphbm_entry_delay(void)
{
	ktime_t cur_timestamp;
	unsigned long diff;
	unsigned long te_time;
	unsigned long delay_time;
	unsigned long delay_threshold;
	unsigned long real_te_interval;

	if (!disp_info->hbm_entry_delay || !disp_info->te_interval_us)
		return;
	real_te_interval = get_real_te_interval();
	delay_threshold = real_te_interval * disp_info->hbm_entry_delay /
		TE_60_TIME;
	LCD_KIT_INFO("delay_threshold = %lu us", delay_threshold);
	if (delay_threshold <= 0)
		return;

	cur_timestamp = ktime_get();
	diff = ktime_to_us(cur_timestamp) -
		ktime_to_us(disp_info->hbm_blcode_ts);
	if (diff >= delay_threshold)
		return;

	te_time = real_te_interval * disp_info->te_interval_us /
		TE_60_TIME;
	delay_time = (delay_threshold - diff) / te_time * te_time + te_time;
	LCD_KIT_INFO("diff = %lu us, TE time = %lu us, delay_time = %lu us",
		diff, te_time, delay_time);
	udelay(delay_time);
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

static void hbm_cmd_tx(struct hbm_type_cfg hbm_source,
	struct lcd_kit_dsi_panel_cmds *pcmds)
{
	if (hbm_source.source == HBM_FOR_FP)
		(void)lcd_kit_fp_highlight_cmds_tx(hbm_source.handle, pcmds);
	else
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(pcmds);
}

static void lcd_kit_set_elvss_dim_fp(int disable_elvss_dim,
	struct hbm_type_cfg hbm_source)
{
	if (!common_info->hbm.hbm_fp_elvss_support) {
		LCD_KIT_INFO("Do not support ELVSS Dim, just return\n");
		return;
	}

	if (common_info->hbm.elvss_prepare_cmds.cmds)
		hbm_cmd_tx(hbm_source, &common_info->hbm.elvss_prepare_cmds);
	if (common_info->hbm.elvss_write_cmds.cmds) {
		if (disable_elvss_dim)
			disable_elvss_dim_write();
		else
			enable_elvss_dim_write();
		hbm_cmd_tx(hbm_source, &common_info->hbm.elvss_write_cmds);
		LCD_KIT_INFO("[fp set elvss dim] send:0x%x\n",
			common_info->hbm.elvss_write_cmds.cmds[0].payload[1]);
	}
	if (common_info->hbm.elvss_post_cmds.cmds)
		hbm_cmd_tx(hbm_source, &common_info->hbm.elvss_post_cmds);
}

static void lcd_kit_fill_hbm_cmd(unsigned int level)
{
	if (common_info->hbm.hbm_special_bit_ctrl ==
		LCD_KIT_HIGH_12BIT_CTL_HBM_SUPPORT) {
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
		common_info->hbm.hbm_cmds.cmds[0].payload[2] = level & 0xff;
	}
}

static void lcd_kit_set_hbm_level_fp(unsigned int level,
	struct hbm_type_cfg hbm_source)
{
	/* prepare */
	if (common_info->hbm.hbm_prepare_cmds.cmds)
		hbm_cmd_tx(hbm_source, &common_info->hbm.hbm_prepare_cmds);
	/* set hbm level */
	if (common_info->hbm.hbm_cmds.cmds) {
		lcd_kit_fill_hbm_cmd(level);
		hbm_cmd_tx(hbm_source, &common_info->hbm.hbm_cmds);
	}
	/* post */
	if (common_info->hbm.hbm_post_cmds.cmds)
		hbm_cmd_tx(hbm_source, &common_info->hbm.hbm_post_cmds);
}

static void lcd_kit_disable_hbm_fp(struct hbm_type_cfg hbm_source)
{
	if (common_info->hbm.exit_dim_cmds.cmds)
		hbm_cmd_tx(hbm_source, &common_info->hbm.exit_dim_cmds);
	if (common_info->hbm.exit_cmds.cmds)
		hbm_cmd_tx(hbm_source, &common_info->hbm.exit_cmds);
}

static void lcd_kit_hbm_set_func_by_level(uint32_t level, int type,
	struct hbm_type_cfg hbm_source)
{
	if (type == LCD_KIT_FP_HBM_ENTER) {
		lcd_kit_set_elvss_dim_fp(LCD_KIT_DISABLE_ELVSSDIM, hbm_source);
		if (common_info->hbm.fp_enter_cmds.cmds)
			hbm_cmd_tx(hbm_source, &common_info->hbm.fp_enter_cmds);
		lcd_kit_set_hbm_level_fp(level, hbm_source);
	} else if (type == LCD_KIT_FP_HBM_EXIT) {
		if (level > 0)
			lcd_kit_set_hbm_level_fp(level, hbm_source);
		else
			lcd_kit_disable_hbm_fp(hbm_source);

		lcd_kit_set_elvss_dim_fp(LCD_KIT_ENABLE_ELVSSDIM, hbm_source);
	} else {
		LCD_KIT_ERR("unknown case!\n");
	}
}

static void lcd_kit_fp_hbm_extern(int type, struct hbm_type_cfg hbm_source)
{
	if (type == LCD_KIT_FP_HBM_ENTER) {
		if (common_info->hbm.fp_enter_extern_cmds.cmds)
			hbm_cmd_tx(hbm_source,
				&common_info->hbm.fp_enter_extern_cmds);
	} else if (type == LCD_KIT_FP_HBM_EXIT) {
		if (common_info->hbm.fp_exit_extern_cmds.cmds)
			hbm_cmd_tx(hbm_source,
				&common_info->hbm.fp_exit_extern_cmds);
	} else {
		LCD_KIT_ERR("unknown case!\n");
	}
}

static void lcd_kit_restore_hbm_level(struct hbm_type_cfg hbm_source)
{
	mutex_lock(&common_info->hbm.hbm_lock);
	lcd_kit_hbm_set_func_by_level(
		common_info->hbm.hbm_level_current,
		LCD_KIT_FP_HBM_EXIT, hbm_source);
	mutex_unlock(&common_info->hbm.hbm_lock);
}

static void lcd_kit_hbm_enter(int level, struct hbm_type_cfg hbm_source)
{
	int hbm_level;
	struct mtk_panel_info *pinfo = &lcd_kit_pinfo;

	if (!common_info->hbm.hbm_fp_support && hbm_source.source == HBM_FOR_FP)
		hbm_level = pinfo->bl_max;
	else
		hbm_level = level;

	if (common_info->hbm.hbm_fp_support && hbm_source.source == HBM_FOR_MMI)
		lcd_kit_mipi_set_backlight(hbm_source, pinfo->bl_max);

	if (common_info->hbm.hbm_fp_support) {
		mutex_lock(&common_info->hbm.hbm_lock);
		if (hbm_source.source == HBM_FOR_FP)
			lcd_kit_fphbm_entry_delay();
		common_info->hbm.hbm_if_fp_is_using = 1;
		lcd_kit_hbm_set_func_by_level(hbm_level,
			LCD_KIT_FP_HBM_ENTER, hbm_source);
		mutex_unlock(&common_info->hbm.hbm_lock);
	} else {
		lcd_kit_mipi_set_backlight(hbm_source, hbm_level);
		lcd_kit_fp_hbm_extern(LCD_KIT_FP_HBM_ENTER, hbm_source);
	}
}

static void lcd_kit_hbm_exit(int level, struct hbm_type_cfg hbm_source)
{
	if (common_info->hbm.hbm_fp_support) {
		lcd_kit_restore_hbm_level(hbm_source);
		mutex_lock(&common_info->hbm.hbm_lock);
		common_info->hbm.hbm_if_fp_is_using = 0;
		mutex_unlock(&common_info->hbm.hbm_lock);
	}
	lcd_kit_mipi_set_backlight(hbm_source, level);
	lcd_kit_fp_hbm_extern(LCD_KIT_FP_HBM_EXIT, hbm_source);
}

static int lcd_kit_set_backlight_by_type(int backlight_type,
	struct hbm_type_cfg hbm_source)
{
	struct mtk_panel_info *pinfo = &lcd_kit_pinfo;

	LCD_KIT_INFO("backlight_type is %d\n", backlight_type);
	if (backlight_type == BACKLIGHT_HIGH_LEVEL) {
		lcd_kit_hbm_enter(common_info->hbm.hbm_level_max, hbm_source);
		disp_info->bl_is_shield_backlight = true;
		lcd_kit_enable_hbm_timer();
		LCD_KIT_INFO("set_bl is max_%d : pre_%d\n",
			pinfo->bl_max, lcd_kit_pinfo.bl_current);
	} else if (backlight_type == BACKLIGHT_LOW_LEVEL) {
		lcd_kit_disable_hbm_timer();
		disp_info->bl_is_shield_backlight = false;
		lcd_kit_hbm_exit(lcd_kit_pinfo.bl_current, hbm_source);
		LCD_KIT_INFO("set_bl is pre_%d : cur_%d\n",
			pinfo->bl_min, lcd_kit_pinfo.bl_current);
	} else {
		LCD_KIT_ERR("bl_type is not define\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}

static void lcd_kit_hbm_disable(void)
{
	if (common_info->hbm.exit_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(&common_info->hbm.exit_cmds);
}

static void lcd_kit_hbm_dim_enable(void)
{
	if (common_info->hbm.enter_dim_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(&common_info->hbm.enter_dim_cmds);
}

static void lcd_kit_hbm_dim_disable(void)
{
	if (common_info->hbm.exit_dim_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(&common_info->hbm.exit_dim_cmds);
}

static void lcd_kit_hbm_set_level(int level)
{
	/* prepare */
	if (common_info->hbm.hbm_prepare_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(
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
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(&common_info->hbm.hbm_cmds);
	}
	/* post */
	if (common_info->hbm.hbm_post_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(&common_info->hbm.hbm_post_cmds);
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

static void lcd_kit_hbm_enable_no_dimming(void)
{
	if (common_info->hbm.enter_no_dim_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(
			&common_info->hbm.enter_no_dim_cmds);
}

static void lcd_kit_hbm_enable(void)
{
	if (common_info->hbm.enter_cmds.cmds != NULL)
		(void)lcd_kit_dsi_cmds_extern_tx_nolock(&common_info->hbm.enter_cmds);
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

static int lcd_kit_hbm_set_handle(int last_hbm_level,
	int hbm_dimming, int hbm_level)
{
	int ret = LCD_KIT_OK;

	if ((hbm_level < 0) || (hbm_level > HBM_SET_MAX_LEVEL)) {
		LCD_KIT_ERR("input param invalid, hbm_level %d!\n", hbm_level);
		return LCD_KIT_FAIL;
	}
	if (!common_info->hbm.support) {
		LCD_KIT_DEBUG("not support hbm\n");
		return ret;
	}
	mutex_lock(&common_info->hbm.hbm_lock);
	common_info->hbm.hbm_level_current = hbm_level;
	if (check_if_fp_using_hbm() < 0) {
		LCD_KIT_INFO("fp is using, exit!\n");
		mutex_unlock(&common_info->hbm.hbm_lock);
		return ret;
	}
	if (hbm_level > 0) {
		if (last_hbm_level == 0) {
			/* enable hbm */
			lcd_kit_hbm_enable();
			if (!hbm_dimming)
				lcd_kit_hbm_enable_no_dimming();
		} else {
			lcd_kit_hbm_print_count(last_hbm_level, hbm_level);
		}
		 /* set hbm level */
		lcd_kit_hbm_set_level(hbm_level);
	} else {
		if (last_hbm_level == 0) {
			/* disable dimming */
			lcd_kit_hbm_dim_disable();
		} else {
			/* exit hbm */
			if (hbm_dimming)
				lcd_kit_hbm_dim_enable();
			else
				lcd_kit_hbm_dim_disable();
			lcd_kit_hbm_disable();
		}
	}
	mutex_unlock(&common_info->hbm.hbm_lock);
	return ret;
}

static int lcd_kit_hbm_set_func(int level, bool dimming)
{
	static int last_hbm_level;
	int ret = LCD_KIT_FAIL;

	if (!common_ops->hbm_set_handle)
		return ret;

	ret = lcd_kit_hbm_set_handle(last_hbm_level, dimming, level);
	last_hbm_level = level;
	return ret;
}

extern void disp_ccorr_clear(enum DISP_MODULE_ENUM module, void *handle);

extern void disp_ccorr_restore(enum DISP_MODULE_ENUM module, void * handle);

extern int primary_display_user_cmd(unsigned int cmd, unsigned long arg);

static int lcd_kit_set_hbm_for_mmi(int level, struct hbm_type_cfg hbm_source)
{
	LCD_KIT_INFO("level = %d\n", level);
	if (level == 0) {
		disp_info->bl_is_shield_backlight = false;
		lcd_kit_hbm_exit(lcd_kit_pinfo.bl_current, hbm_source);
		return LCD_KIT_OK;
	}
	lcd_kit_hbm_enter(level, hbm_source);
	disp_info->bl_is_shield_backlight = true;
	return LCD_KIT_OK;
}

static void handle_ccorr_for_hbm(int level)
{
	if (level == 0)
	{
		LCD_KIT_INFO("mtk_drm_ccorr_restore + \n");
		primary_display_user_cmd(DISP_IOCTL_RESTORE_CCORR, NULL);
		LCD_KIT_INFO("mtk_drm_ccorr_restore - \n");
	} else {
		LCD_KIT_INFO("mtk_drm_ccorr_clear + \n");
		primary_display_user_cmd(DISP_IOCTL_CLEAR_CCORR, NULL);
		LCD_KIT_INFO("mtk_drm_ccorr_clear - \n");
	}
}

extern int primary_display_manual_lock_ex(void);
extern int primary_display_manual_unlock_ex(void);

int panel_drm_hbm_set(struct drm_device *dev, void *data,
	struct drm_file *file_priv)
{
	int ret = LCD_KIT_FAIL;
	struct display_engine_ddic_hbm_param *hbm_cfg =
		(struct display_engine_ddic_hbm_param *)data;
	struct hbm_type_cfg hbm_source = {
		.source = HBM_FOR_MMI,
	};

	if (!data) {
		LCD_KIT_ERR("data is null\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("alpm_state:%d\n", disp_info->alpm_state);
	if (disp_info->alpm_state != ALPM_OUT) {
		LCD_KIT_INFO("in alpm state, return!\n");
		return LCD_KIT_FAIL;
	}


	LCD_KIT_INFO("hbm tpye:%d, level:%d, dimming:%d\n", hbm_cfg->type,
		hbm_cfg->level, hbm_cfg->dimming);
	if (hbm_cfg->type == HBM_FOR_MMI)
		handle_ccorr_for_hbm(hbm_cfg->level);
	(void)primary_display_manual_lock_ex();
	if (hbm_cfg->type == HBM_FOR_MMI)
		ret = lcd_kit_set_hbm_for_mmi(hbm_cfg->level, hbm_source);

	if (hbm_cfg->type == HBM_FOR_LIGHT)
		ret = lcd_kit_hbm_set_func(hbm_cfg->level, hbm_cfg->dimming);
	(void)primary_display_manual_unlock_ex();

	return ret;
}

static bool lcm_get_hbm_state(void)
{
	return hbm_en;
}

static bool lcm_get_hbm_wait(void)
{
	return hbm_wait;
}

static bool lcm_set_hbm_wait(bool wait)
{
	bool old = hbm_wait;

	hbm_wait = wait;
	return old;
}

static bool lcm_set_hbm_cmdq(bool en, void *qhandle)
{
	bool old = hbm_en;
	struct hbm_type_cfg hbm_source = {
		HBM_FOR_FP, NULL, NULL, qhandle };

	if (hbm_en == en)
		goto done;
	if (disp_info->alpm_state != ALPM_OUT) {
		LCD_KIT_INFO("in alpm state, return!\n");
		return old;
	}

	if (en)
		(void)lcd_kit_set_backlight_by_type(BACKLIGHT_HIGH_LEVEL,
			hbm_source);
	else
		(void)lcd_kit_set_backlight_by_type(BACKLIGHT_LOW_LEVEL,
			hbm_source);

	hbm_en = en;
	lcm_set_hbm_wait(true);

done:
	return old;
}

static void lcm_disable(void)
{
	LCD_KIT_INFO(" +!\n");
	if (!disp_info->pre_power_off)
		return;
	if (common_info->hbm.support)
		common_info->hbm.hbm_level_current = 0;

	if (disp_info->bl_is_shield_backlight == true)
		disp_info->bl_is_shield_backlight = false;
	lcd_kit_disable_hbm_timer();
	if (common_ops->panel_power_off)
		common_ops->panel_power_off(NULL);
	lcm_set_panel_state(LCD_POWER_STATE_OFF);
	disp_info->alpm_state = ALPM_OUT;
	hbm_en = false;
	LCD_KIT_INFO(" -!\n");
}
#endif

struct LCM_DRIVER lcdkit_mtk_common_panel = {
	.panel_info = &lcd_kit_pinfo,
	.name = "lcdkit_mtk_common_panel_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcd_kit_on,
	.suspend = lcd_kit_off,
	.resume = lcm_resume,
	.set_backlight_cmdq = lcd_kit_set_backlight,
#ifdef CONFIG_MACH_MT6768
	.set_hbm_cmdq = lcm_set_hbm_cmdq,
	.get_hbm_state = lcm_get_hbm_state,
	.get_hbm_wait = lcm_get_hbm_wait,
	.set_hbm_wait = lcm_set_hbm_wait,
	.set_aod_area_cmdq = lcm_set_aod_area,
	.aod = lcm_aod,
	.get_doze_delay = lcm_get_doze_delay,
	.disable = lcm_disable,
#endif
};

static int __init lcd_kit_init(void)
{
	int ret = LCD_KIT_OK;
	struct device_node *np = NULL;

	LCD_KIT_INFO("enter\n");

	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return ret;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("not find device node %s!\n", DTS_COMP_LCD_KIT_PANEL_TYPE);
		ret = -1;
		return ret;
	}

	OF_PROPERTY_READ_U32_RETURN(np, "product_id", &disp_info->product_id);
	LCD_KIT_INFO("disp_info->product_id = %d\n", disp_info->product_id);
	disp_info->compatible = (char *)of_get_property(np, "lcd_panel_type", NULL);
	if (!disp_info->compatible) {
		LCD_KIT_ERR("can not get lcd kit compatible\n");
		return ret;
	}
	LCD_KIT_INFO("disp_info->compatible: %s\n", disp_info->compatible);

	np = of_find_compatible_node(NULL, NULL, disp_info->compatible);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", disp_info->compatible);
		ret = -1;
		return ret;
	}

#if defined CONFIG_HUAWEI_DSM
	lcd_dclient = dsm_register_client(&dsm_lcd);
#endif
	mutex_init(&disp_info->drm_hw_lock);
	/* 1.adapt init */
	lcd_kit_adapt_init();
	bias_bl_ops_init();
	/* 2.common init */
	if (common_ops->common_init)
		common_ops->common_init(np);
	/* 3.utils init */
	lcd_kit_utils_init(np, lcdkit_mtk_common_panel.panel_info);
	/* 4.init fnode */
	lcd_kit_sysfs_init();
	/* 5.init factory mode */
	/* 6.power init */
	lcd_kit_power_init();
	/* 7.init panel ops */
	lcd_kit_panel_init();
	/* get lcd max brightness */
	lcd_kit_get_bl_max_nit_from_dts();
	lcd_kit_set_thp_proximity_state(POWER_ON);
#ifdef CONFIG_MACH_MT6768
	hbm_en = false;
#endif
	if (common_ops->btb_init) {
		common_ops->btb_init();
		if (common_info->btb_check_type == LCD_KIT_BTB_CHECK_GPIO)
			common_ops->btb_check();
	}
	LCD_KIT_INFO("exit\n");
	return ret;
}

fs_initcall(lcd_kit_init);
#endif
