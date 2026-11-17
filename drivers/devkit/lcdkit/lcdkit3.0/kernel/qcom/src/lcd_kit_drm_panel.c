/*
 * lcd_kit_drm_panel.c
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
#include "lcd_kit_drm_panel.h"
#ifdef CONFIG_LOG_JANK
#include <huawei_platform/log/hwlog_kernel.h>
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

struct dsm_client *lcd_dclient = NULL;
#endif

unsigned int esd_recovery_level = 0;
static struct qcom_panel_info lcd_kit_info = {0};
static struct lcd_kit_disp_info g_lcd_kit_disp_info;

extern int dsi_panel_set_pinctrl_state(struct dsi_panel *panel, bool enable);
extern int dsi_panel_tx_cmd_set(struct dsi_panel *panel,
				enum dsi_cmd_set_type type);
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

struct qcom_panel_info *lcm_get_panel_info(void)
{
	return &lcd_kit_info;
}

int is_mipi_cmd_panel(void)
{
	if (lcd_kit_info.panel_dsi_mode == 0)
		return 1;
	return 0;
}

int  lcd_kit_bl_ic_set_backlight(unsigned int bl_level)
{
	struct lcd_kit_bl_ops *bl_ops = NULL;

	bl_ops = lcd_kit_get_bl_ops();
	if (!bl_ops) {
		LCD_KIT_INFO("bl_ops is null!\n");
		return LCD_KIT_FAIL;
	}
	if (bl_ops->set_backlight) {
		if (bl_level > 0)
			esd_recovery_level = bl_level;
		bl_ops->set_backlight(bl_level);
	}
	return LCD_KIT_OK;
}

int lcd_kit_dsi_panel_update_backlight(struct dsi_panel *panel,
	unsigned int level)
{
	ssize_t ret = 0;
	struct mipi_dsi_device *dsi = NULL;
	unsigned char bl_tb_short[] = { 0xFF };
	unsigned char bl_tb_long[] = { 0xFF, 0xFF };

	if (!panel || (level > 0xffff)) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}
	dsi = &panel->mipi_device;
	switch (common_info->backlight.order) {
	case BL_BIG_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] =
				(level >> 8) & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] =
				level & 0xFF;
		}
		break;
	case BL_LITTLE_ENDIAN:
		if (common_info->backlight.bl_max <= 0xFF) {
			common_info->backlight.bl_cmd.cmds[0].payload[1] = level;
		} else {
			/* change bl level to dsi cmds */
			common_info->backlight.bl_cmd.cmds[0].payload[1] =
				level & 0xFF;
			common_info->backlight.bl_cmd.cmds[0].payload[2] =
				(level >> 8) & 0xFF;
		}
		break;
	default:
		LCD_KIT_ERR("not support order\n");
		break;
	}
	if(common_info->backlight.bl_max <= 0xFF) {
		bl_tb_short[0] = common_info->backlight.bl_cmd.cmds[0].payload[1];
		ret = mipi_dsi_dcs_write(dsi,
			common_info->backlight.bl_cmd.cmds[0].payload[0],
			bl_tb_short, sizeof(bl_tb_short));
	} else {
		bl_tb_long[0] = common_info->backlight.bl_cmd.cmds[0].payload[1];
		bl_tb_long[1] = common_info->backlight.bl_cmd.cmds[0].payload[2];
		ret = mipi_dsi_dcs_write(dsi,
			common_info->backlight.bl_cmd.cmds[0].payload[0],
			bl_tb_long, sizeof(bl_tb_long));
	}
	if (ret < 0)
		LCD_KIT_ERR("failed to update dcs backlight:%d\n", level);
	return ret;
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

void lcm_set_panel_state(unsigned int state)
{
	lcd_kit_info.panel_state = state;
}

unsigned int lcm_get_panel_state(void)
{
	return lcd_kit_info.panel_state;
}

unsigned int lcm_get_panel_backlight_max_level(void)
{
	return lcd_kit_info.bl_max;
}

int lcm_rgbw_mode_set_param(struct drm_device *dev, void *data,
	struct drm_file *file_priv)
{
	int ret = LCD_KIT_OK;
	struct display_engine_ddic_rgbw_param *param = NULL;

	if (dev == NULL) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (data == NULL) {
		LCD_KIT_ERR("data is null\n");
		return LCD_KIT_FAIL;
	}
	param = (struct display_engine_ddic_rgbw_param *)data;
	memcpy(&g_lcd_kit_disp_info.ddic_rgbw_param, param, sizeof(*param));
	ret = lcd_kit_rgbw_set_handle();
	if (ret < 0)
		LCD_KIT_ERR("set rgbw fail\n");
	return ret;
}

int lcm_rgbw_mode_get_param(struct drm_device *dev, void *data,
	struct drm_file *file_priv)
{
	int ret = LCD_KIT_OK;
	struct display_engine_ddic_rgbw_param *param = NULL;

	if (dev == NULL) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (data == NULL) {
		LCD_KIT_ERR("data is null\n");
		return LCD_KIT_FAIL;
	}
	param = (struct display_engine_ddic_rgbw_param *)data;
	param->ddic_panel_id = g_lcd_kit_disp_info.ddic_rgbw_param.ddic_panel_id;
	param->ddic_rgbw_mode = g_lcd_kit_disp_info.ddic_rgbw_param.ddic_rgbw_mode;
	param->ddic_rgbw_backlight = g_lcd_kit_disp_info.ddic_rgbw_param.ddic_rgbw_backlight;
	param->pixel_gain_limit = g_lcd_kit_disp_info.ddic_rgbw_param.pixel_gain_limit;

	LCD_KIT_INFO("get RGBW parameters success\n");
	return ret;
}

int lcm_display_engine_get_panel_info(struct drm_device *dev, void *data,
	struct drm_file *file_priv)
{
	int ret = LCD_KIT_OK;
	struct display_engine_panel_info_param *param = NULL;

	if (dev == NULL) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (data == NULL) {
		LCD_KIT_ERR("data is null\n");
		return LCD_KIT_FAIL;
	}
	param = (struct display_engine_panel_info_param *)data;
	param->width = lcd_kit_info.xres;
	param->height = lcd_kit_info.yres;
	param->maxluminance = lcd_kit_info.maxluminance;
	param->minluminance = lcd_kit_info.minluminance;
	param->maxbacklight = lcd_kit_info.bl_max;
	param->minbacklight = lcd_kit_info.bl_min;

	LCD_KIT_INFO("get panel info parameters success\n");
	return ret;
}

int lcm_display_engine_init(struct drm_device *dev, void *data,
	struct drm_file *file_priv)
{
	int ret = LCD_KIT_OK;
	struct display_engine *param = NULL;

	if (dev == NULL) {
		LCD_KIT_ERR("dev is null\n");
		return LCD_KIT_FAIL;
	}
	if (data == NULL) {
		LCD_KIT_ERR("data is null\n");
		return LCD_KIT_FAIL;
	}
	param = (struct display_engine *)data;
	/* 0:no support  1:support */
	if (g_lcd_kit_disp_info.rgbw.support == 0)
		param->ddic_rgbw_support = 0;
	else
		param->ddic_rgbw_support = 1;

	LCD_KIT_INFO("display engine init success\n");
	return ret;
}

static int lcd_kit_dsi_panel_power_on(struct dsi_panel *panel)
{
	int ret = 0;

	if (panel == NULL) {
		LCD_KIT_ERR("panel is null\n");
		return ret;
	}
	lcd_kit_set_thp_proximity_sem(true);
	ret = dsi_panel_set_pinctrl_state(panel, true);
	if (ret)
		LCD_KIT_ERR("[%s] failed to set pinctrl, rc=%d\n", panel->name, ret);

	if (common_ops->panel_power_on)
		ret = common_ops->panel_power_on(panel);
	lcm_set_panel_state(LCD_POWER_STATE_ON);
	lcd_kit_set_thp_proximity_state(POWER_ON);
	lcd_kit_set_thp_proximity_sem(false);
	return ret;
}

static int lcd_kit_dsi_panel_on_lp(struct dsi_panel *panel)
{
	int ret = 0;

	if (panel == NULL) {
		LCD_KIT_ERR("panel is null\n");
		return ret;
	}

	if (common_ops->panel_on_lp)
		ret = common_ops->panel_on_lp(panel);

	return ret;
}

static int lcd_kit_dsi_panel_power_off(struct dsi_panel *panel)
{
	int ret = 0;

	if (panel == NULL) {
		LCD_KIT_ERR("panel is null\n");
		return ret;
	}
	lcd_kit_set_thp_proximity_sem(true);
	if (common_ops->panel_power_off)
		common_ops->panel_power_off(panel);
	ret = dsi_panel_set_pinctrl_state(panel, false);
	if (ret)
		LCD_KIT_ERR("[%s] failed set pinctrl state, rc=%d\n", panel->name,
			ret);
	lcm_set_panel_state(LCD_POWER_STATE_OFF);
	lcd_kit_set_thp_proximity_state(POWER_OFF);
	lcd_kit_set_thp_proximity_sem(false);
#ifdef CONFIG_LOG_JANK
	LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "LCD_POWER_OFF");
#endif
	return ret;
}

static int lcd_kit_dsi_panel_off_lp(struct dsi_panel *panel)
{
	int ret = 0;

	if (panel == NULL) {
		LCD_KIT_ERR("panel is null\n");
		return ret;
	}

	if (common_ops->panel_off_lp)
		ret = common_ops->panel_off_lp(panel);

	return ret;
}

static int lcd_kit_dsi_panel_off_hs(struct dsi_panel *panel)
{
	int ret = 0;

	if (panel == NULL) {
		LCD_KIT_ERR("panel is null\n");
		return LCD_KIT_FAIL;
	}

	if (common_ops->panel_off_hs)
		ret = common_ops->panel_off_hs(panel);

	return ret;
}

int lcd_kit_panel_pre_prepare(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	/* If LP11_INIT is set, panel will be powered up during prepare() */
	if (panel->lp11_init)
		goto error;

	rc = lcd_kit_dsi_panel_power_on(panel);
	if (rc) {
		LCD_KIT_ERR("[%s] panel power on failed, rc=%d\n", panel->name, rc);
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	LCD_KIT_INFO("exit\n");
	return rc;
}

int lcd_kit_panel_prepare(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	if (panel->lp11_init) {
		rc = lcd_kit_dsi_panel_power_on(panel);
		if (rc) {
			LCD_KIT_ERR("[%s] panel power on failed, rc=%d\n",
			       panel->name, rc);
			goto error;
		}
	}

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_PRE_ON);
	if (rc) {
		LCD_KIT_ERR("[%s] failed to send DSI_CMD_SET_PRE_ON cmds, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	LCD_KIT_INFO("exit\n");
	return rc;
}

int lcd_kit_panel_enable(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	lcd_kit_proxmity_proc(LCD_RESET_HIGH);
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ON);
	if (rc)
		LCD_KIT_ERR("[%s] failed to send DSI_CMD_SET_ON cmds, rc=%d\n",
		       panel->name, rc);
	else
		panel->panel_initialized = true;
	/* record panel on time */
	if (disp_info->quickly_sleep_out.support)
		lcd_kit_disp_on_record_time();
	rc = lcd_kit_dsi_panel_on_lp(panel);
	if (rc != LCD_KIT_OK)
		LCD_KIT_ERR("panel on lp failed\n");
	mutex_unlock(&panel->panel_lock);
#ifdef CONFIG_LOG_JANK
	LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
#endif
	LCD_KIT_INFO("exit\n");
	return rc;
}

int lcd_kit_panel_pre_disable(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = lcd_kit_dsi_panel_off_hs(panel);
	if (rc != LCD_KIT_OK)
		LCD_KIT_INFO("panel off hs failed\n");

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_PRE_OFF);
	if (rc) {
		LCD_KIT_ERR("[%s] failed to send DSI_CMD_SET_PRE_OFF cmds, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int lcd_kit_panel_disable(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	/* Avoid sending panel off commands when ESD recovery is underway */
	if (!atomic_read(&panel->esd_recovery_pending)) {
		/*
		 * Need to set IBB/AB regulator mode to STANDBY,
		 * if panel is going off from AOD mode.
		 */
		if (dsi_panel_is_type_oled(panel) &&
			(panel->power_mode == SDE_MODE_DPMS_LP1 ||
			panel->power_mode == SDE_MODE_DPMS_LP2))
			dsi_pwr_panel_regulator_mode_set(&panel->power_info,
				"ibb", REGULATOR_MODE_STANDBY);
		rc = lcd_kit_dsi_panel_off_lp(panel);
		if (rc != LCD_KIT_OK)
			LCD_KIT_ERR("panel off lp failed\n");
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_OFF);
		if (rc) {
			/*
			 * Sending panel off commands may fail when  DSI
			 * controller is in a bad state. These failures can be
			 * ignored since controller will go for full reset on
			 * subsequent display enable anyway.
			 */
			pr_warn_ratelimited("[%s] failed to send DSI_CMD_SET_OFF cmds, rc=%d\n",
					panel->name, rc);
			rc = 0;
		}
	}
	panel->panel_initialized = false;
	panel->power_mode = SDE_MODE_DPMS_OFF;

	mutex_unlock(&panel->panel_lock);
	LCD_KIT_INFO("exit\n");
	return rc;
}

int lcd_kit_panel_unprepare(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_POST_OFF);
	if (rc)
		LCD_KIT_ERR("[%s] failed to send DSI_CMD_SET_POST_OFF cmds, rc=%d\n",
		       panel->name, rc);
	mutex_unlock(&panel->panel_lock);
	LCD_KIT_INFO("exit\n");
	return rc;
}

int lcd_kit_panel_post_unprepare(struct dsi_panel *panel)
{
	int rc = 0;

	LCD_KIT_INFO("enter\n");
	if (!panel) {
		LCD_KIT_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	rc = lcd_kit_dsi_panel_power_off(panel);
	if (rc)
		LCD_KIT_ERR("[%s] panel power_Off failed, rc=%d\n",
		       panel->name, rc);
	mutex_unlock(&panel->panel_lock);
	LCD_KIT_INFO("exit\n");
	return rc;
}

int lcd_kit_init(struct dsi_panel *panel)
{
	int ret = LCD_KIT_OK;
	struct device_node *np = NULL;

	LCD_KIT_INFO("enter\n");
	if (!lcd_kit_support()) {
		LCD_KIT_INFO("not lcd_kit driver and return\n");
		return ret;
	}

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_LCD_KIT_PANEL_TYPE);
	if (np == NULL) {
		LCD_KIT_ERR("not find device node %s\n", DTS_COMP_LCD_KIT_PANEL_TYPE);
		return LCD_KIT_FAIL;
	}

	OF_PROPERTY_READ_U32_RETURN(np, "product_id", &disp_info->product_id);
	LCD_KIT_INFO("product_id = %d\n", disp_info->product_id);

	if (panel == NULL) {
		LCD_KIT_ERR("panel is null\n");
		return LCD_KIT_FAIL;
	}

	if (panel->panel_of_node == NULL) {
		LCD_KIT_ERR("not found device node\n");
		return LCD_KIT_FAIL;
	}
#if defined CONFIG_HUAWEI_DSM
	lcd_dclient = dsm_register_client(&dsm_lcd);
#endif
	/* adapt init */
	lcd_kit_adapt_init();
	/* common init */
	if (common_ops->common_init)
		common_ops->common_init(panel->panel_of_node);
	/* utils init */
	lcd_kit_utils_init(panel->panel_of_node, &lcd_kit_info);
	/* init fnode */
	lcd_kit_sysfs_init();
	/* power init */
	lcd_kit_power_init(panel);
	/* init panel ops */
	lcd_kit_panel_init();
	/* get lcd max brightness */
	lcd_kit_get_bl_max_nit_from_dts();
	lcm_set_panel_state(LCD_POWER_STATE_ON);
	lcd_kit_set_thp_proximity_state(POWER_ON);

	LCD_KIT_INFO("exit\n");
	return ret;
}
