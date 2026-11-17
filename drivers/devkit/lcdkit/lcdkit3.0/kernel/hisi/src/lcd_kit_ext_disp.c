/*
 * lcd_kit_ext_display.c
 *
 * lcdkit display function for lcd driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "lcd_kit_ext_disp.h"
#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_dbg.h"
#include <huawei_platform/log/log_jank.h>
#include "global_ddr_map.h"
#include "lcd_kit_utils.h"
#include "lcd_kit_adapt.h"
#include "lcd_kit_power.h"
#include "lcd_kit_parse.h"
#ifdef LV_GET_LCDBK_ON
#include <huawei_platform/inputhub/sensor_feima_ext.h>
#endif
#include <linux/hisi/hw_cmdline_parse.h>

struct lcd_public_config g_lcd_public_cfg = {
	LCD_NORMAL_TYPE,
	LCD_MAIN_PANEL,
	LCD_MAIN_PANEL,
	1,
	true,
};

struct lcd_pwr_off_optimize_config g_pwr_off_optimize_cfg;

struct lcd_public_config *lcd_kit_get_public_config(void)
{
	return &g_lcd_public_cfg;
}

struct hisi_panel_info *lcd_kit_get_fold_panel_info(
	struct platform_device *pdev, uint32_t panel_id)
{
	if ((PUBLIC_CFG->product_type != LCD_FOLDER_TYPE)
		|| (panel_id >= LCD_ACTIVE_PANEL_BUTT)) {
		LCD_KIT_ERR("panel id err %u.\n", panel_id);
		return NULL;
	}
	return lcd_kit_get_pinfo(panel_id);
}

int lcd_kit_get_product_type(void)
{
	struct hisi_panel_info *panel_info = NULL;

	panel_info = lcd_kit_get_pinfo(LCD_ACTIVE_PANEL);
	if (!panel_info)
		return LCD_NORMAL_TYPE;
	return panel_info->product_type;
}

void lcd_kit_panel_switch(uint32_t panel_id)
{
	if (PUBLIC_CFG->product_type != LCD_FOLDER_TYPE) {
		LCD_KIT_ERR("lcd folder type is %u.\n",
			PUBLIC_CFG->product_type);
		return;
	}

	if (panel_id >= LCD_ACTIVE_PANEL_BUTT) {
		LCD_KIT_ERR("lcd folder type err %u.\n", panel_id);
		return;
	}
	LCD_KIT_INFO("lcd folder type from %u change to %u.\n",
		PUBLIC_CFG->active_panel_id, panel_id);
	PUBLIC_CFG->active_panel_id = panel_id;
}

int lcd_kit_panel_switch_func(struct platform_device *pdev, uint32_t panel_id)
{
	lcd_kit_panel_switch(panel_id);
	return LCD_KIT_OK;
}
bool lcd_kit_need_add_device(uint32_t panel_id)
{
	LCD_KIT_INFO("product_type is %u, boot panel id %u.\n",
		PUBLIC_CFG->product_type,
		PUBLIC_CFG->cmdline_panel_id);

	if (PUBLIC_CFG->product_type == LCD_NORMAL_TYPE) {
		return true;
	} else if (PUBLIC_CFG->product_type == LCD_FOLDER_TYPE) {
		if (panel_id == PUBLIC_CFG->cmdline_panel_id)
			return true;
		else
			return false;
	}
	return true;
}
void lcd_kit_set_default_active_panel(void)
{
	PUBLIC_CFG->active_panel_id = PUBLIC_CFG->cmdline_panel_id;
	LCD_KIT_INFO("active panel id %u.\n",
		PUBLIC_CFG->active_panel_id);
}
void lcd_kit_set_probe_defer(uint32_t panel_id)
{
	if (PUBLIC_CFG->product_type != LCD_FOLDER_TYPE)
		return;
	if (panel_id != PUBLIC_CFG->cmdline_panel_id)
		PUBLIC_CFG->lcd_probe_defer = false;
	lcd_kit_set_default_active_panel();
}
bool lcd_kit_probe_defer(uint32_t panel_id)
{
	LCD_KIT_INFO("panel_type is %u, cmdline panel id %u.\n",
		PUBLIC_CFG->product_type,
		PUBLIC_CFG->cmdline_panel_id);
	if (PUBLIC_CFG->product_type != LCD_FOLDER_TYPE)
		return false;
	if (PUBLIC_CFG->cmdline_panel_id == panel_id)
		return PUBLIC_CFG->lcd_probe_defer;
	return false;
}
static int __init early_parse_lcd_folder_type_cmdline(char *arg)
{
	char lcd_folder_str[LCD_FOLDER_STR] = {0};

	if (!arg) {
		LCD_KIT_ERR("parse elvss dim, arg is NULL\n");
		return LCD_KIT_FAIL;
	}

	strncpy(lcd_folder_str, arg, sizeof(lcd_folder_str) - 1);

	LCD_KIT_INFO("lcd_folder_type parse from cmdline: %s\n",
		lcd_folder_str);
	if (strcmp(lcd_folder_str, "near") == 0)
		PUBLIC_CFG->cmdline_panel_id = LCD_EXT_PANEL;
	else if (strcmp(lcd_folder_str, "far") == 0)
		PUBLIC_CFG->cmdline_panel_id = LCD_MAIN_PANEL;
	return 0;
}

early_param("androidboot.foldertype", early_parse_lcd_folder_type_cmdline);

static void lcd_kit_power_off_optimize_work(struct work_struct *work)
{
	int ret;
	unsigned int i;
	struct lcd_kit_array_data *pevent = NULL;

	LCD_KIT_INFO("power off work start!\n");

	(void *)work;
	if (!g_pwr_off_optimize_cfg.working_flag) {
		LCD_KIT_INFO("work state is err!\n");
		return;
	}
	pevent = g_pwr_off_optimize_cfg.panel_power_off_event;
	for (i = 0; i < g_pwr_off_optimize_cfg.panel_power_off_seq_cnt; i++) {
		if (!pevent || !pevent->buf) {
			LCD_KIT_ERR("pevent is null!\n");
			g_pwr_off_optimize_cfg.working_flag = false;
			return;
		}
		ret = lcd_kit_event_handler_plugin(g_pwr_off_optimize_cfg.hisifd,
			pevent->buf[EVENT_NUM],
			pevent->buf[EVENT_DATA],
			pevent->buf[EVENT_DELAY]);
		if (ret) {
			LCD_KIT_ERR("send event 0x%x error!\n",
				pevent->buf[EVENT_NUM]);
			g_pwr_off_optimize_cfg.working_flag = false;
			break;
		}
		pevent++;
	}
	g_pwr_off_optimize_cfg.working_flag = false;
	return;
}

void lcd_kit_fold_aod_power_on_handle(struct hisi_fb_data_type *hisifd)
{
	struct lcd_kit_power_seq *aod_power_seq = NULL;
	struct lcd_kit_power_desc *aod_power_hdl = NULL;
	struct hisi_panel_info *pinfo = &(hisifd->panel_info);
	uint32_t aod_panel_id;

	if (pinfo->product_type != LCD_FOLDER_TYPE)
		return;
	if (!lcd_kit_get_panel_power_on_status(hisifd))
		return;
	if (g_pwr_off_optimize_cfg.working_flag) {
		LCD_KIT_ERR("last power off work still running!\n");
		return;
	}
	aod_panel_id = hisi_aod_get_panel_id();
	if (aod_panel_id == pinfo->disp_panel_id) {
		LCD_KIT_INFO("aod panel is the same as the cur panel!\n");
		return;
	}
	LCD_KIT_INFO("aod panel is %u, cur panel is %u!\n",
		aod_panel_id, pinfo->disp_panel_id);
	lcd_kit_clear_sctrl_reg(hisifd);
	aod_power_seq = lcd_kit_get_panel_power_seq(aod_panel_id);
	aod_power_hdl = lcd_kit_get_panel_power_handle(aod_panel_id);
	if (!aod_power_seq || !aod_power_hdl) {
		LCD_KIT_ERR("aod_power_seq or aod_power_hdl is null!\n");
		return;
	}
	LCD_KIT_INFO("aod power off work init.\n");
	g_pwr_off_optimize_cfg.working_flag = true;
	g_pwr_off_optimize_cfg.hisifd = hisifd;
	g_pwr_off_optimize_cfg.panel_power_off_seq_cnt =
		aod_power_seq->power_off_seq.cnt;
	g_pwr_off_optimize_cfg.panel_power_off_event =
		aod_power_seq->power_off_seq.arry_data;
	g_pwr_off_optimize_cfg.lcd_iovcc = &aod_power_hdl->lcd_iovcc;
	g_pwr_off_optimize_cfg.lcd_vci = &aod_power_hdl->lcd_vci;
	g_pwr_off_optimize_cfg.lcd_vdd = &aod_power_hdl->lcd_vdd;
	g_pwr_off_optimize_cfg.lcd_rst = &aod_power_hdl->lcd_rst;

	lcd_kit_power_off_optimize_work(NULL);
}

void lcd_kit_power_off_optimize_handle(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = &(hisifd->panel_info);

	if (!disp_info->pwr_off_optimize_info.support)
		return;
	if (pinfo->product_type != LCD_FOLDER_TYPE)
		return;
	if (pinfo->skip_power_on_off != FOLD_POWER_ON_OFF)
		return;
	if (g_pwr_off_optimize_cfg.working_flag) {
		LCD_KIT_ERR("last power off work still running!\n");
		cancel_delayed_work_sync(&g_pwr_off_optimize_cfg.pwr_off_optimize_work);
		lcd_kit_power_off_optimize_work(NULL);
	}
	LCD_KIT_INFO("power off work init.\n");
	g_pwr_off_optimize_cfg.working_flag = true;
	g_pwr_off_optimize_cfg.hisifd = hisifd;
	g_pwr_off_optimize_cfg.panel_power_off_seq_cnt =
		power_seq->power_off_seq.cnt;
	g_pwr_off_optimize_cfg.panel_power_off_event =
		power_seq->power_off_seq.arry_data;
	g_pwr_off_optimize_cfg.lcd_iovcc = &power_hdl->lcd_iovcc;
	g_pwr_off_optimize_cfg.lcd_vci = &power_hdl->lcd_vci;
	g_pwr_off_optimize_cfg.lcd_vdd = &power_hdl->lcd_vdd;
	g_pwr_off_optimize_cfg.lcd_rst = &power_hdl->lcd_rst;

	schedule_delayed_work(&g_pwr_off_optimize_cfg.pwr_off_optimize_work,
		msecs_to_jiffies(disp_info->pwr_off_optimize_info.delay_time));
}

static void lcd_kit_register_power_off_optimize(void)
{
	struct delayed_work *pwr_off_optimize_work = NULL;

	pwr_off_optimize_work = &g_pwr_off_optimize_cfg.pwr_off_optimize_work;
	g_pwr_off_optimize_cfg.working_flag = false;
	INIT_DELAYED_WORK(pwr_off_optimize_work, lcd_kit_power_off_optimize_work);
	LCD_KIT_INFO("power off work regist!\n");
}

static int lcd_kit_ext_panel_probe(struct platform_device *pdev)
{
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	int ret;

	np = pdev->dev.of_node;
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("enter probe!\n");
	if (lcd_kit_probe_defer(LCD_EXT_PANEL)) {
		LCD_KIT_ERR("lcd kit probe defer\n");
		goto err_probe_defer;
	}
	lcd_kit_panel_switch(LCD_EXT_PANEL);
	pinfo = lcd_kit_get_pinfo(LCD_EXT_PANEL);
	if (!pinfo) {
		LCD_KIT_ERR("pinfo is null\n");
		return LCD_KIT_FAIL;
	}
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->disp_panel_id = LCD_EXT_PANEL;
	/* adapt init */
	lcd_kit_adapt_init();
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
	/* probe driver */
	if (hisi_fb_device_probe_defer(pinfo->type, pinfo->bl_set_type))
		goto err_probe_defer;
	if (lcd_kit_need_add_device(LCD_EXT_PANEL)) {
		pdev->id = 1; /* dev id should be 1 */
		lcd_kit_pdata()->panel_info = pinfo;
		ret = platform_device_add_data(pdev, lcd_kit_pdata(),
			sizeof(struct hisi_fb_panel_data));
		if (ret) {
			LCD_KIT_ERR("platform_device_add_data failed!\n");
			goto err_device_put;
		}
		LCD_KIT_ERR("lcd_kit_need_add_device LCD_EXT_PANEL!\n");
		hisi_fb_add_device(pdev);
	}

#ifdef LCD_FACTORY_MODE
	lcd_factory_init(np);
#endif
	lcd_kit_register_power_key_notify();
	lcd_kit_register_power_off_optimize();
	/* init fnode */
	lcd_kit_sysfs_init();
	lcd_kit_set_probe_defer(LCD_EXT_PANEL);
	LCD_KIT_INFO("exit probe!\n");
	return LCD_KIT_OK;

err_device_put:
	platform_device_put(pdev);
err_probe_defer:
	return -EPROBE_DEFER;
}

static struct of_device_id lcd_kit_ext_panel_match_table[] = {
	{
		.compatible = "lcd_ext_panel_default",
		.data = NULL,
	},
	{},
};

static struct platform_driver lcd_kit_ext_panel_driver = {
	.probe = lcd_kit_ext_panel_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "lcd_kit_ext_mipi_panel",
		.of_match_table = lcd_kit_ext_panel_match_table,
	},
};

void lcd_kit_ext_panel_init(void)
{
	int ret;
	int len;
	unsigned int floder_type = 0;
	struct device_node *np = NULL;

	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return;
	}
	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n",
			DTS_COMP_LCD_KIT_PANEL_TYPE);
		return;
	}
	OF_PROPERTY_READ_U32_RETURN(np, "product_type",
		&floder_type);
	PUBLIC_CFG->product_type = floder_type;
	LCD_KIT_INFO("disp_info->folder_type = 0x%x\n",
		PUBLIC_CFG->product_type);
	if (PUBLIC_CFG->product_type != LCD_FOLDER_TYPE)
		return;

	lcd_kit_panel_switch(LCD_EXT_PANEL);
	if (of_property_read_u32(np, "board_version",
		&disp_info->board_version))
		disp_info->board_version = 0;

	LCD_KIT_INFO("disp_info->board_version = 0x%x\n",
		disp_info->board_version);
	lcd_kit_parse_u32(np, "product_id", &disp_info->product_id, 0);
	LCD_KIT_INFO("disp_info->product_id = %d",
		disp_info->product_id);
	disp_info->compatible = (char *)of_get_property(np,
		"ext_lcd_panel_type", NULL);
	if (!disp_info->compatible) {
		LCD_KIT_ERR("can not get lcd kit compatible\n");
		return;
	}
	LCD_KIT_INFO("disp_info->compatible = %s\n", disp_info->compatible);
	len = strlen(disp_info->compatible);
	memset((char *)lcd_kit_ext_panel_driver.driver.of_match_table->compatible,
		0, LCD_KIT_PANEL_COMP_LENGTH);
	strncpy((char *)lcd_kit_ext_panel_driver.driver.of_match_table->compatible,
		disp_info->compatible, len > (LCD_KIT_PANEL_COMP_LENGTH - 1) ?
		(LCD_KIT_PANEL_COMP_LENGTH - 1) : len);
	/* register driver */
	ret = platform_driver_register(&lcd_kit_ext_panel_driver);
	if (ret)
		LCD_KIT_ERR("Driver_register failed, error = %d!\n", ret);
}
