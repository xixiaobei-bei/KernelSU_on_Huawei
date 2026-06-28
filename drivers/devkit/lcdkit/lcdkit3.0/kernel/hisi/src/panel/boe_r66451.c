/*
*panel adapter
*product:laya
*panel:boe_r66451
*/

#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_utils.h"

extern int g_min_backlight_from_app;
extern int g_max_backlight_from_app;
static struct timer_list backlight_second_timer;
static struct lcd_kit_disp_info g_lcd_kit_disp_info;

#define LEVEL1 121
#define LEVEL2 233
#define LEVEL3 534
#define LEVEL4 3975
#define LEVEL1_NEW 121
#define LEVEL2_NEW 672
#define LEVEL3_NEW 1653
#define LEVEL4_NEW 3976
#define FRAME_TIME 16666

static void lcd_kit_second_timerout_function(unsigned long arg)
{
	unsigned long temp;
	temp = arg;
	if(disp_info->bl_is_shield_backlight == true) {
		disp_info->bl_is_shield_backlight = false;
	}
	del_timer(&backlight_second_timer);
	disp_info->bl_is_start_second_timer = false;
	LCD_KIT_INFO("Sheild backlight 1.2s timeout, remove the backlight sheild.\n");
}

static int boe_r66451_set_backlight_by_type(struct platform_device* pdev, int backlight_type)
{
	int ret = 0;
	int max_backlight = 0;
	int min_backlight = 0;
	struct hisi_fb_data_type* hisifd = NULL;
	struct lcd_kit_adapt_ops* adapt_ops = NULL;
	int bl_delay_time = 0;
	int level1 = 0;
	int level2 = 0;
	int level3 = 0;
	int level4 = 0;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (NULL == pdev) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	hisifd = platform_get_drvdata(pdev);

	if (NULL == hisifd) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("backlight_type is %d\n", backlight_type);

	max_backlight = g_max_backlight_from_app;
	min_backlight = g_min_backlight_from_app;
	hisifb_display_effect_fine_tune_backlight(hisifd, g_min_backlight_from_app, (int*)&min_backlight);
	bl_delay_time = FRAME_TIME * (hisifd->panel_info.bl_delay_frame);

	LCD_KIT_INFO("panel_version is %s\n", hisifd->panel_info.lcd_panel_version);

	if (!strcmp(hisifd->panel_info.lcd_panel_version, " VER:VN1")
		||!strcmp(hisifd->panel_info.lcd_panel_version, " VER:V4")){
		level1 = LEVEL1;
		level2 = LEVEL2;
		level3 = LEVEL3;
		level4 = LEVEL4;
	} else {
		level1 = LEVEL1_NEW;
		level2 = LEVEL2_NEW;
		level3 = LEVEL3_NEW;
		level4 = LEVEL4_NEW;
	}
	switch (backlight_type) {
	case BACKLIGHT_HIGH_LEVEL:
		udelay(bl_delay_time);
		if (common_info->hbm.prepare_cmds_fir.cmds != NULL) {
			if (adapt_ops->mipi_tx) {
				//LEVEL1 = LEVELMIN
				if(min_backlight >= level1 && min_backlight < level2){
					ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.prepare_cmds_fir);
				}
				else if (min_backlight >= level2 && min_backlight < level3){
						ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.prepare_cmds_sec);
				}
				else if (min_backlight >= level3 && min_backlight < level4){
					ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.prepare_cmds_thi);
				}
				else if(min_backlight == level4){
					ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.prepare_cmds_fou);
				}
				else {
					LCD_KIT_INFO("backlight level is too high or too low when fingerprint.\n");
				}
			}
		}

		disp_info->bl_is_shield_backlight = true;
		if(disp_info->bl_is_start_second_timer == false) {
			init_timer(&backlight_second_timer);
			backlight_second_timer.expires = jiffies + 12*HZ/10;// 1.2s
			backlight_second_timer.data = 0;
			backlight_second_timer.function = lcd_kit_second_timerout_function;
			add_timer(&backlight_second_timer);
			disp_info->bl_is_start_second_timer = true;
		} else {
			//if timer is not timeout, restart timer
			mod_timer(&backlight_second_timer, (jiffies + 12*HZ/10));// 1.2s
		}
		LCD_KIT_INFO("backlight_type is (%d), set_backlight is (%d)\n", backlight_type, max_backlight);
		break;
	case BACKLIGHT_LOW_LEVEL:
		if(disp_info->bl_is_start_second_timer == true) {
			del_timer(&backlight_second_timer);
			disp_info->bl_is_start_second_timer = false;
		}
		disp_info->bl_is_shield_backlight = false;
		udelay(bl_delay_time);
		if (common_info->hbm.exit_cmds_fir.cmds != NULL) {
			if (adapt_ops->mipi_tx) {
				if(min_backlight >= level1 && min_backlight < level2){
					ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_cmds_fir);
				}
				else if (min_backlight >= level2 && min_backlight < level3){
					ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_cmds_sec);
				}
				else if (min_backlight >= level3 && min_backlight < level4){
					if (!strcmp(hisifd->panel_info.lcd_panel_version, " VER:VN1")
						|| (!strcmp(hisifd->panel_info.lcd_panel_version, " VER:V4"))){
						ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_cmds_thi);
					} else {
						ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_cmds_thi_new);
					}
				}
				else if(min_backlight == level4){
					ret = adapt_ops->mipi_tx((void *)hisifd, &common_info->hbm.exit_cmds_fou);
				}
				else {
					LCD_KIT_INFO("backlight level is too high or too low when fingerprint.\n");
				}
			}
		}

		LCD_KIT_INFO("backlight_type is (%d), set_backlight is (%d)\n", backlight_type, min_backlight);
		break;
	default:
		LCD_KIT_ERR("backlight_type is not define(%d).\n", backlight_type);
		break;
	}
	return ret;
}


static struct lcd_kit_panel_ops boe_r66451_ops = {
	.lcd_kit_set_backlight_by_type = boe_r66451_set_backlight_by_type,
};

int boe_r66451_probe(void)
{
	int ret = LCD_KIT_OK;

	ret = lcd_kit_panel_ops_register(&boe_r66451_ops);
	if (ret) {
		LCD_KIT_ERR("failed\n");
		return LCD_KIT_FAIL;
	}
	return LCD_KIT_OK;
}
