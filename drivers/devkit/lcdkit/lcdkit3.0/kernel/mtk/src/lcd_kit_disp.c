/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define LOG_TAG "LCM"

#include "lcm_drv.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_common.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_core.h"
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
/*#include <mach/mt_pm_ldo.h>*/
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
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

struct LCM_UTIL_FUNCS lcm_util_mtk;

static struct mtk_panel_info lcd_kit_pinfo = {0};

static struct lcd_kit_disp_info g_lcd_kit_disp_info;
struct lcd_kit_disp_info *lcd_kit_get_disp_info(void)
{
	return &g_lcd_kit_disp_info;
}

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util_mtk, util, sizeof(struct LCM_UTIL_FUNCS));  
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
	params->physical_width_um = pinfo->width * 1000;
	params->physical_height_um = pinfo->height * 1000;

	params->dsi.mode = pinfo->panel_dsi_mode;
	params->dsi.switch_mode = pinfo->panel_dsi_switch_mode;
	params->dsi.switch_mode_enable = 0;
    params->density = pinfo->panel_density;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = pinfo->mipi.lane_nums;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = pinfo->bgr_fmt;
	params->dsi.data_format.trans_seq = pinfo->panel_trans_seq;
	params->dsi.data_format.padding = pinfo->panel_data_padding;
	params->dsi.data_format.format = pinfo->bpp;

	/* Highly depends on LCD driver capability. */
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

	params->dsi.PLL_CLOCK = pinfo->pxl_clk_rate;//440;	/* this value must be in MTK suggested table */
	params->dsi.fbk_div =  pinfo->pxl_fbk_div;
	params->dsi.CLK_HS_POST = pinfo->mipi.clk_post_adjust;
	params->dsi.clk_lp_per_line_enable = pinfo->mipi.lp11_flag;
	params->dsi.esd_check_enable = pinfo->esd_enable;
	params->dsi.customization_esd_check_enable = 0;

}

static void lcd_kit_on(void)
{
	LCD_KIT_INFO(" +!\n");
    lcd_kit_pinfo.panel_state = 1;

	if (common_ops->panel_power_on) {
		common_ops->panel_power_on((void*)NULL);
	}

	/*record panel on time*/
	lcd_kit_disp_on_record_time();

	LCD_KIT_INFO(" -!\n");
	return;
}

static void lcd_kit_off(void)
{
    lcd_kit_pinfo.panel_state = 0;

	LCD_KIT_INFO(" +!\n");

	if (common_ops->panel_power_off) {
		common_ops->panel_power_off(NULL);
	}

	LCD_KIT_INFO(" -!\n");
}

static void lcm_resume(void)
{
	lcd_kit_on();
}

static void lcd_kit_set_backlight(void *handle, unsigned int level)
{
    int ret = 0;

	LCD_KIT_INFO("%s, backlight: level = %d\n", __func__, level);

	ret = common_ops->set_mipi_backlight(NULL, level);
    if (ret < 0){
        return;
    }
}

struct LCM_DRIVER lcdkit_mtk_common_panel = {
    .panel_info = &lcd_kit_pinfo,
	.name = "lcdkit_mtk_common_panel_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcd_kit_on,
	.suspend = lcd_kit_off,
	.resume = lcm_resume,
	.set_backlight_cmdq = lcd_kit_set_backlight,
};

static int __init lcd_kit_init(void)
{
	int ret = LCD_KIT_OK;
	struct device_node* np = NULL;

	LCD_KIT_INFO(" +!\n");

	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return ret;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_LCD_KIT_PANEL_TYPE);
		ret = -1;
		return ret;
	}

	OF_PROPERTY_READ_U32_RETURN(np, "product_id", &disp_info->product_id);
	LCD_KIT_INFO("disp_info->product_id = %d\n", disp_info->product_id);
	disp_info->compatible = (char*)of_get_property(np, "lcd_panel_type", NULL);
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

	/*1.adapt init*/
	lcd_kit_adapt_init();
	/*2.common init*/
	if (common_ops->common_init) {
		common_ops->common_init(np);
	}
	/*3.utils init*/
	lcd_kit_utils_init(np, lcdkit_mtk_common_panel.panel_info);
	/*4.init fnode*/
	lcd_kit_sysfs_init();
	/*5.init factory mode*/
	//lcd_kit_factory_init(pinfo);
	/*6.power init*/
	lcd_kit_power_init();
	/*7.init panel ops*/
	lcd_kit_panel_init();
	return ret;
}

fs_initcall(lcd_kit_init);

