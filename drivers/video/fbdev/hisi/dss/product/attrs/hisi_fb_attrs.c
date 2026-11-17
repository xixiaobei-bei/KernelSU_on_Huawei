/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 */
/*lint -e559*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "hisi_fb.h"
#include "hisi_display_effect.h"
#include "hisi_dpe_utils.h"


#ifdef CONFIG_LCD_KIT_DRIVER
#include "lcd_kit_core.h"
#endif

const uint32_t max_buf = 60;

static ssize_t dpufb_frame_update_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int val;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	static uint32_t esd_enable;

	if (dev == NULL) {
		DPU_FB_ERR("frame update store dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("frame update store fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("frame update store dpufd NULL Pointer\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("frame update store buf NULL Pointer\n");
		return -1;
	}

	val = (int)simple_strtoul(buf, NULL, 0);

	DPU_FB_INFO("fb%d, val=%d\n", dpufd->index, val);

	down(&dpufd->blank_sem);

	g_enable_dirty_region_updt = (val > 0) ? 0 : 1;
	dpufd->frame_update_flag = (val > 0) ? 1 : 0;
	dpufb_set_vsync_activate_state(dpufd, (val > 0) ? true : false);

	if (!is_mipi_cmd_panel(dpufd))
		goto err_out;

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	dpufb_activate_vsync(dpufd);
	if (val == 1) {
		esd_enable = dpufd->panel_info.esd_enable;
		dpufd->panel_info.esd_enable = 0;
		mdelay(50);
	}

	ldi_frame_update(dpufd, (val > 0) ? true : false);
	if (val == 0) {
		dpufd->vactive0_start_flag = 1;
		mdelay(50);
		dpufd->panel_info.esd_enable = esd_enable;
		esd_enable = 0;
	}
	dpufb_deactivate_vsync(dpufd);

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_frame_update_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("frame_update_show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("frame_update_show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("frame_update_show dpufd NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("frame_update_show buf NULL Pointer\n");
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n",
		dpufd->vsync_ctrl.vsync_infinite);
}

static ssize_t dpufb_lcd_model_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd model show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd model show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd model show dpufd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd model show dpufd pdata NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd model show dpufd buf NULL Pointer\n");
		return -1;
	}

	if (pdata->lcd_model_show != NULL)
		ret = pdata->lcd_model_show(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_lcd_cabc_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd cabc mode show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd cabc mode show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd cabc mode show dpufd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd cabc mode show  pdata NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd cabc mode show buf NULL Pointer\n");
		return -1;
	}

	if (pdata->lcd_cabc_mode_show != NULL)
		ret = pdata->lcd_cabc_mode_show(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_lcd_cabc_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd cabc mode store dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd cabc mode store fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd cabc mode store dpufd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd cabc mode store pdata NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd cabc mode store buf NULL Pointer\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_cabc_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_cabc_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_ce_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd ce mode show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd ce mode show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd ce mode show dpufd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd ce mode show pdata NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd ce mode show buf NULL Pointer\n");
		return -1;
	}

	if (pdata->lcd_ce_mode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_ce_mode_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

	return ret;
}

static ssize_t dpufb_lcd_ce_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd ce mode store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd ce mode store fbi NULL Pointer\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd ce mode store dpufd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd ce mode store pdata NULL Pointer\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd ce mode store buf NULL Pointer\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_ce_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_ce_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd ce mode store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_check_reg_show(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd check reg show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd check reg show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd check reg show dpufd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd check reg show pdata NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd check reg show buf NULL Pointer\n");
		return -1;
	}


	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata != NULL && pdata->lcd_check_reg) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_check_reg(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	} else {
		DPU_FB_ERR("lcd_check_reg is NULL\n");
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_frame_count_show(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("frame_count_show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("frame_count_show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("frame_count_show dpufd NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("frame_count_show buf NULL Pointer\n");
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n", dpufd->frame_count);
}

static ssize_t dpufb_lcd_mipi_detect_show(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd mipi detect show dev NULL Pointer\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd mipi detect show fbi NULL Pointer\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd mipi detect show dpufd NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata != NULL && pdata->lcd_mipi_detect) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_mipi_detect(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	} else {
		DPU_FB_ERR("lcd_mipi_detect is NULL\n");
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_mipi_dsi_bit_clk_upt_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("mipi_dsi_bit_clk_upt_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("mipi_dsi_bit_clk_upt_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("mipi_dsi_bit_clk_upt_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("mipi_dsi_bit_clk_upt_show pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("mipi_dsi_bit_clk_upt_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->mipi_dsi_bit_clk_upt_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->mipi_dsi_bit_clk_upt_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_mipi_dsi_bit_clk_upt_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("mipi dsi bit clk upt store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("mipi dsi bit clk upt store fbi NULL Pointer\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("mipi dsi bit clk upt store dpufd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->mipi_dsi_bit_clk_upt_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->mipi_dsi_bit_clk_upt_store(
			dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("mipi dsi bit clk upt store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_panel_mode_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("fbi NULL Pointer\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd NULL Pointer\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return ret;
	}
	ret = panel_mode_switch_store(dpufd, buf, count);
	if (ret < 0)
		DPU_FB_INFO("panel_mode_switch_store ret is %d\n", ret);

	return count;
}

static ssize_t dpufb_lcd_fps_scence_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd fps scence show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd fps scence show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd fps scence show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd fps scence show dpufd NULL Pointer!\n");
		return -1;
	}

	ret = snprintf(buf, PAGE_SIZE, "lcd_fps = %d\n",
			dpufd->panel_info.fps);

	return ret;
}

static ssize_t dpufb_lcd_fps_scence_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	uint32_t val;

	if (dev == NULL) {
		DPU_FB_ERR("lcd fps scence store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd fps scence store fbi NULL Pointer\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd fps scence store dpufd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd fps scence store pdata NULL Pointer!\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd fps scence store buf NULL Pointer!\n");
		return ret;
	}

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		return ret;
	}

	val = (uint32_t)simple_strtoul(buf, NULL, 0);

	if (pdata->lcd_fps_scence_handle != NULL) {
		ret = pdata->lcd_fps_scence_handle(dpufd->pdev, val);
		if (ret < 0)
			DPU_FB_INFO("lcd fps scence store ret is %d\n", ret);
	}

	return count;
}

static ssize_t dpufb_lcd_hkadc_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_hkadc_debug_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_hkadc_debug_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_hkadc_debug_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_hkadc_debug_show pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd_hkadc_debug_show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_hkadc_debug_show != NULL)
		ret = pdata->lcd_hkadc_debug_show(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_lcd_hkadc_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd hkadc debug store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd hkadc debug store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd hkadc debug store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd hkadc debug store pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd hkadc debug store buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_hkadc_debug_store != NULL)
		ret = pdata->lcd_hkadc_debug_store(dpufd->pdev, buf, count);

	return count;
}

static ssize_t dpufb_lcd_gram_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_gram_check_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_gram_check_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_gram_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd gram check store dev  NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd gram check store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd gram check store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd gram check store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd gram check store buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_gram_check_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_gram_check_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd gram check store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_dynamic_sram_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_dynamic_sram_check_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_dynamic_sram_checksum_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_dynamic_sram_checksum_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_dynamic_sram_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd dynamic sram check store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd dynamic sram check store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd dynamic sram check store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd dynamic sram check store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd dynamic sram check store buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_dynamic_sram_checksum_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_dynamic_sram_checksum_store(
			dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd dynamic sram check store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}


static ssize_t dpufb_lcd_color_temperature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_color_temperature_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_color_temperature_show dev fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_color_temperature_show dev dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_color_temperature_show dev pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_color_temperature_show dev buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_color_temperature_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_color_temperature_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_color_temperature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd color temperature store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd color temperature store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd color temperature store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd color temperature store pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd color temperature store  buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_color_temperature_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_color_temperature_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_ic_color_enhancement_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_ic_color_enhancement_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_ic_color_enhancement_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_ic_color_enhancement_mode_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_ic_color_enhancement_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_ic_color_enhancement_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_ic_color_enhancement_mode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_ic_color_enhancement_mode_show(
			dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_ic_color_enhancement_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd ic color enhancement mode store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd ic color enhancement mode store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd ic color enhancement mode store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd ic color enhancement mode store pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd ic color enhancement mode store buf NULL Pointer!\n");
		return -1;
	}


	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_ic_color_enhancement_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_ic_color_enhancement_mode_store(dpufd->pdev,
			buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_led_rg_lcd_color_temperature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR(
			"led_rg_lcd_color_temperature_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR(
			"led_rg_lcd_color_temperature_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR(
			"led_rg_lcd_color_temperature_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR(
			"led_rg_lcd_color_temperature_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR(
			"led_rg_lcd_color_temperature_show buf NULL Pointer!\n");
		return -1;
	}


	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->led_rg_lcd_color_temperature_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->led_rg_lcd_color_temperature_show(
				dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_led_rg_lcd_color_temperature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("led rg lcd color temperature store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("led rg lcd color temperature store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("led rg lcd color temperature store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("led rg lcd color temperature store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("led rg lcd color temperature store buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->led_rg_lcd_color_temperature_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->led_rg_lcd_color_temperature_store(dpufd->pdev,
			buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("led rg lcd color temperature store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_support_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_support_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_support_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_support_mode_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_support_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_support_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_support_mode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_support_mode_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

	// support none if lower lcd_support_mode_show
	// functions have not be returned
	if (ret == 0) {
		DPU_FB_WARNING("fb%d, support none!\n", dpufd->index);
		ret = snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}
err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_support_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd support mode store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd support mode store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd support mode store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd support mode store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd support mode store buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_support_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_support_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}
err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd support mode store dev ret is %d\n", ret);

	up(&dpufd->blank_sem);
	return count;
}

static ssize_t dpufb_lcd_comform_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_comform_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_comform_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_comform_mode_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_comform_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_comform_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_comform_mode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_comform_mode_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_comform_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd comform mode store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd comform mode store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd comform mode store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd comform mode store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd comform mode store bufNULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		dpe_update_g_comform_discount(0);
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_comform_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_comform_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd comform mode store ret is %d\n", ret);

	up(&dpufd->blank_sem);
	return count;
}

static ssize_t dpufb_lcd_cinema_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_cinema_mode_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_cinema_mode_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_cinema_mode_show hisfd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_cinema_mode_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_cinema_mode_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_cinema_mode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_cinema_mode_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_cinema_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd cinema mode store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd cinema mode store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd cinema mode store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd cinema mode store padata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd cinema mode store buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_cinema_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_cinema_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);
	return count;
}

static ssize_t dpufb_lcd_voltage_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd voltage enable store dev NULL Pointer\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd voltage enable store fbi NULL Pointer\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd voltage enable store dpufd NULL Pointer\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer\n");
		return ret;
	}


	down(&dpufd->blank_sem);

	if (pdata->lcd_voltage_enable_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_voltage_enable_store(dpufd->pdev, buf, count);
		if (ret < 0)
			DPU_FB_INFO(
				"lcd voltage enable store ret is %d\n", ret);

		dpufb_deactivate_vsync(dpufd);
	}

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_bist_check(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	char lcd_bist_check_result[512] = {0};

	if (dev == NULL) {
		DPU_FB_ERR("lcd bist check dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd bist check fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd bist check dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd bist check pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd bist check buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_bist_check != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_bist_check(dpufd->pdev,
			lcd_bist_check_result);
		dpufb_deactivate_vsync(dpufd);
	}

	ret = snprintf(buf, PAGE_SIZE, "%s", lcd_bist_check_result);
	DPU_FB_INFO("LCD bist check result : %s\n",
		lcd_bist_check_result);

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_func_switch_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_panel_info *pinfo = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd func switch show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd func switch show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd func switch show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd func switch show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd func switch show buf NULL Pointer!\n");
		return -1;
	}

	pinfo = &(dpufd->panel_info);

	ret = snprintf(buf, PAGE_SIZE,
		"xcc_support=%d\n"
		"dsi_bit_clk_upt=%d\n"
		"dirty_region_upt=%d\n"
		"fps_updt_support=%d\n"
		"ifbc_type=%d\n"
		"esd_enable=%d\n"
		"blpwm_input_ena=%d\n"
		"blpwm_precision_type=%d\n"
		"lane_nums=%d\n"
		"panel_effect_support=%d\n"
		"color_temp_rectify_support=%d\n"
		"hiace=%d\n"
		"effect_enable=%d\n"
		"effect_debug=%d\n",
		pinfo->xcc_support,
		pinfo->dsi_bit_clk_upt_support,
		g_enable_dirty_region_updt,
		pinfo->fps_updt_support,
		pinfo->ifbc_type,
		pinfo->esd_enable,
		pinfo->blpwm_input_ena,
		pinfo->blpwm_precision_type,
		pinfo->mipi.lane_nums + 1,
		pinfo->panel_effect_support,
		pinfo->color_temp_rectify_support,
		pinfo->hiace_support,
		g_enable_effect,
		g_debug_effect);

	return ret;
}

static u32 xcc_table_def[12] = {
	0x0, 0x8000, 0x0, 0x0, 0x0, 0x0, 0x8000,
	0x0, 0x0, 0x0, 0x0, 0x8000, };

static void dpufb_lcd_func_switch_store_xcc_support(
	struct dpu_panel_info *pinfo, const char *command)
{
	if (pinfo == NULL) {
		DPU_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (command == NULL) {
		DPU_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("xcc_support:", command, strlen("xcc_support:"))) {
		if (command[strlen("xcc_support:")] == '0') {
			pinfo->xcc_support = 0;
			if (pinfo->xcc_table != NULL) {
				pinfo->xcc_table[1] = 0x8000;
				pinfo->xcc_table[6] = 0x8000;
				pinfo->xcc_table[11] = 0x8000;
			}
			DPU_FB_INFO("xcc_support disable\n");
		} else {
			pinfo->xcc_support = 1;
			if (pinfo->xcc_table == NULL) {
				pinfo->xcc_table = xcc_table_def;
				pinfo->xcc_table_len =
					ARRAY_SIZE(xcc_table_def);
			}
			DPU_FB_INFO("xcc_support enable\n");
		}
	}
}

static void dpufb_lcd_func_switch_store_blpwm(struct dpu_panel_info *pinfo,
	const char *command)
{
	if (pinfo == NULL) {
		DPU_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (command == NULL) {
		DPU_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("blpwm_input_ena:", command, strlen("blpwm_input_ena:"))) {
		if ('0' == command[strlen("blpwm_input_ena:")]) {
			pinfo->blpwm_input_ena = 0;
			DPU_FB_INFO("blpwm_input_ena disable\n");
		} else {
			pinfo->blpwm_input_ena = 1;
			DPU_FB_INFO("blpwm_input_ena enable\n");
		}
	}

	if (!strncmp("blpwm_precision_type:", command,
		strlen("blpwm_precision_type:"))) {
		if (command[strlen("blpwm_precision_type:")] == '0') {
			pinfo->blpwm_precision_type = 0;
			DPU_FB_INFO("blpwm_precision_type default\n");
		} else {
			pinfo->blpwm_precision_type =
				BLPWM_PRECISION_10000_TYPE;
			DPU_FB_INFO("BLPWM_PRECISION_10000_TYPE\n");
		}
	}
}

static int dpufb_lcd_func_switch_store_lane_nums(
	struct dpu_fb_data_type *dpufd, struct dpu_panel_info *pinfo,
		const char *command)
{
	if (pinfo == NULL) {
		DPU_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return 0;
	}

	if (command == NULL) {
		DPU_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return 0;
	}

	if (!strncmp("lane_nums:", command, strlen("lane_nums:"))) {
		if (dpufd->panel_power_on) {
			DPU_FB_ERR(
				"fb%d, lane_nums can't be changed when panel power on!\n",
					dpufd->index);
			return -1;
		}

		if ((command[strlen("lane_nums:")] == '1') &&
			(pinfo->mipi.lane_nums_select_support &
			DSI_1_LANES_SUPPORT)) {
			pinfo->mipi.lane_nums = DSI_1_LANES;
			DPU_FB_INFO("lane_nums: DSI_1_LANES\n");
		} else if ((command[strlen("lane_nums:")] == '2') &&
			(pinfo->mipi.lane_nums_select_support &
			DSI_2_LANES_SUPPORT)) {
			pinfo->mipi.lane_nums = DSI_2_LANES;
			DPU_FB_INFO("lane_nums: DSI_2_LANES\n");
		} else if ((command[strlen("lane_nums:")] == '3') &&
			(pinfo->mipi.lane_nums_select_support &
				DSI_3_LANES_SUPPORT)) {
			pinfo->mipi.lane_nums = DSI_3_LANES;
			DPU_FB_INFO("lane_nums: DSI_3_LANES\n");
		} else {
			pinfo->mipi.lane_nums = DSI_4_LANES;
			DPU_FB_INFO("lane_nums: DSI_4_LANES\n");
		}
	}
	return 0;
}

static void dpufb_lcd_func_switch_store_esd_fps(
	struct dpu_panel_info *pinfo, const char *command)
{
	if (pinfo == NULL) {
		DPU_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (command == NULL) {
		DPU_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("esd_enable:", command, strlen("esd_enable:"))) {
		if (command[strlen("esd_enable:")] == '0') {
			pinfo->esd_enable = 0;
			DPU_FB_INFO("esd_enable disable\n");
		} else {
			pinfo->esd_enable = 1;
			DPU_FB_INFO("esd_enable enable\n");
		}
	}

	if (!strncmp("fps_updt_support:", command,
		strlen("fps_updt_support:"))) {
		if (command[strlen("fps_updt_support:")] == '0') {
			pinfo->fps_updt_support = 0;
			DPU_FB_INFO("fps_updt_support disable\n");
		} else {
			pinfo->fps_updt_support = 1;
			DPU_FB_INFO("fps_updt_support enable\n");
		}
	}
}

static void dpufb_lcd_func_switch_store_lcd_info(
	struct dpu_panel_info *pinfo, const char *command)
{
	if (pinfo == NULL) {
		DPU_FB_ERR("lcd func switch store pinfo NULL Pointer!\n");
		return;
	}

	if (command == NULL) {
		DPU_FB_ERR("lcd func switch store command NULL Pointer!\n");
		return;
	}

	if (!strncmp("dsi_bit_clk_upt:", command, strlen("dsi_bit_clk_upt:"))) {
		if (command[strlen("dsi_bit_clk_upt:")] == '0') {
			pinfo->dsi_bit_clk_upt_support = 0;
			DPU_FB_INFO("dsi_bit_clk_upt disable\n");
		} else {
			pinfo->dsi_bit_clk_upt_support = 1;
			DPU_FB_INFO("dsi_bit_clk_upt enable\n");
		}
	}

	if (!strncmp("dirty_region_upt:", command,
		strlen("dirty_region_upt:"))) {
		if (command[strlen("dirty_region_upt:")] == '0') {
			g_enable_dirty_region_updt = 0;
			DPU_FB_INFO("dirty_region_upt disable\n");
		} else {
			g_enable_dirty_region_updt = 1;
			DPU_FB_INFO("dirty_region_upt enable\n");
		}
	}

	if (!strncmp("ifbc_type:", command, strlen("ifbc_type:"))) {
		if (command[strlen("ifbc_type:")] == '0') {
			if (pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE) {
				// ldi
				pinfo->ldi.h_back_porch *=
					pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_front_porch *=
					pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_pulse_width *=
					pinfo->pxl_clk_rate_div;

				pinfo->pxl_clk_rate_div = 1;
				pinfo->ifbc_type = IFBC_TYPE_NONE;
				DPU_FB_INFO(
					"ifbc_type changed to IFBC_TYPE_NONE\n");
			}
		} else if (command[strlen("ifbc_type:")] == '7') {
			if (pinfo->ifbc_type == IFBC_TYPE_NONE) {
				pinfo->pxl_clk_rate_div = 3;

				// ldi
				pinfo->ldi.h_back_porch /=
					pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_front_porch /=
					pinfo->pxl_clk_rate_div;
				pinfo->ldi.h_pulse_width /=
					pinfo->pxl_clk_rate_div;

				pinfo->ifbc_type = IFBC_TYPE_VESA3X_SINGLE;
				DPU_FB_INFO(
					"ifbc_type changed to IFBC_TYPE_VESA3X_SINGLE\n");
			}
		}
	}

	dpufb_lcd_func_switch_store_esd_fps(pinfo, command);
}

static ssize_t dpufb_lcd_func_switch_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_panel_info *pinfo = NULL;
	char command[max_buf] = {0};
	ssize_t ret;

	if (dev == NULL) {
		DPU_FB_ERR("lcd func switch store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd func switch store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd func switch store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd func switch store pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd func switch store buf NULL Pointer!\n");
		return -1;
	}

	if (strlen(buf) >= max_buf) {
		DPU_FB_ERR("buf overflow!\n");
		return -1;
	}

	pinfo = &(dpufd->panel_info);
	// cppcheck-suppress *
	ret = sscanf(buf, "%s", command);
	if (!ret) {
		DPU_FB_INFO("bad command(%s)\n", command);
		return count;
	}

	down(&dpufd->blank_sem);

	dpufb_activate_vsync(dpufd);

	dpufb_lcd_func_switch_store_xcc_support(pinfo, command);
	dpufb_lcd_func_switch_store_lcd_info(pinfo, command);
	dpufb_lcd_func_switch_store_blpwm(pinfo, command);

	if (dpufb_lcd_func_switch_store_lane_nums(dpufd, pinfo, command) < 0)
		goto out;

	if (!strncmp("panel_effect_support:", command,
		strlen("panel_effect_support:"))) {
		if (command[strlen("panel_effect_support:")] == '0') {
			pinfo->panel_effect_support = 0;
			DPU_FB_INFO("panel_effect_support disable\n");
		} else {
			pinfo->panel_effect_support = 1;
			DPU_FB_INFO("panel_effect_support enable\n");
		}
	}

	if (!strncmp("color_temp_rectify_support:", command,
		strlen("color_temp_rectify_support:"))) {
		if (command[strlen("color_temp_rectify_support:")] == '0') {
			pinfo->color_temp_rectify_support = 0;
			DPU_FB_INFO("color_temp_rect disable\n");
		} else {
			pinfo->color_temp_rectify_support = 1;
			DPU_FB_INFO("color_temp_rect enable\n");
		}
	}

	dpufb_display_effect_func_switch(dpufd, command);

out:
	dpufb_deactivate_vsync(dpufd);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_sleep_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (pdata->lcd_sleep_ctrl_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_sleep_ctrl_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_sleep_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd sleep ctrl store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd sleep ctrl store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd sleep ctrl store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd sleep ctrl store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd sleep ctrl store buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_sleep_ctrl_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_sleep_ctrl_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd sleep ctrl store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_effect_al_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_al_show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_al_show buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_al_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t dpufb_effect_al_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_al_store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_al_store buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_al_ctrl_store(dev_get_drvdata(dev),
		buf, count);
}

static ssize_t dpufb_effect_ce_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void *)dev;
	(void *)attr;
	(void *)buf;
	return 0;
}

static ssize_t dpufb_effect_ce_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void *)dev;
	(void *)attr;
	(void *)buf;
	return (ssize_t)count;
} //lint !e715

static ssize_t dpufb_effect_hdr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_hdr_mode_show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_hdr_mode_show buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_ce_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t dpufb_effect_hdr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_hdr_mode_store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_hdr_mode_store buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_ce_ctrl_store(dev_get_drvdata(dev),
		buf, count);
}

static ssize_t dpufb_effect_bl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_bl_show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_bl_show buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_bl_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t dpufb_effect_bl_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_bl_enable_show deb NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_bl_enable_show buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_bl_enable_ctrl_show(
		dev_get_drvdata(dev), buf);
}

static ssize_t dpufb_effect_bl_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_bl_enable_store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_bl_enable_store buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_bl_enable_ctrl_store(
		dev_get_drvdata(dev), buf, count);
}

static ssize_t dpufb_effect_sre_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_sre_show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_sre_show buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_sre_ctrl_show(dev_get_drvdata(dev), buf);
}

static ssize_t dpufb_effect_sre_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	(void)attr;

	if (dev == NULL) {
		DPU_FB_ERR("effect_sre_store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("effect_sre_store buf NULL Pointer!\n");
		return -1;
	}

	return dpufb_display_effect_sre_ctrl_store(
		dev_get_drvdata(dev), buf, count);
}

static ssize_t dpufb_effect_metadata_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("effect metadata show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("effect metadata show fbi NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("effect metadata show buf NULL Pointer!\n");
		return -1;
	}

	return 0;
}

static ssize_t dpufb_effect_metadata_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("effect metadata store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("effect metadata store fbi NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("effect metadata store buf NULL Pointer!\n");
		return -1;
	}

	return 0;
}

static ssize_t dpufb_effect_available_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/*
	 * bit0:hiace; bit1:csc; bit2:bitextend;
	 * bit3:dither; bit4:arsr1p; bit6:acm; bit7:igm;
	 * bit8:xcc; bit9:gmp; bit10:gamma
	 */
	int value;
	struct fb_info *fbi = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("effect available show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("effect available show fbi NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("effect available show buf NULL Pointer!\n");
		return -1;
	}

	value = 0;

	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

static ssize_t dpufb_lcd_test_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd_test_config_show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_test_config_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_test_config_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_test_config_show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd_test_config_show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (pdata->lcd_test_config_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_test_config_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_support_checkmode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd support checkmode show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd support checkmode show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR(
			"lcd support checkmode show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd support checkmode show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd support checkmode show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_support_checkmode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_support_checkmode_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

	return ret;
}

static ssize_t dpufb_lcd_test_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd test config store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd test config store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd test config store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd test config store pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd test config store buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (pdata->lcd_test_config_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_test_config_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_reg_read_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd reg read show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd reg read show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd reg read show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd reg read show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd reg read show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_reg_read_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_reg_read_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_reg_read_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd reg read store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd reg read store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd reg read store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd reg read store pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd reg read store buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_reg_read_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_reg_read_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_lp2hs_mipi_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check show pdata NULL Pointer!\n");
		return -1;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check show buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (pdata->lcd_lp2hs_mipi_check_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_lp2hs_mipi_check_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_lp2hs_mipi_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR(
			"lcd lp2hs mipi check store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check store pdata NULL Pointer!\n");
		return ret;
	}
	if (buf == NULL) {
		DPU_FB_ERR("lcd lp2hs mipi check store buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (pdata->lcd_lp2hs_mipi_check_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_lp2hs_mipi_check_store(dpufd->pdev,
			buf, count);
		if (ret < 0)
			DPU_FB_INFO(
				"lcd lp2hs mipi check store ret is %d\n", ret);

		dpufb_deactivate_vsync(dpufd);
	}

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_inversion_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd inversion store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd inversion store  fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd inversion store  dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd inversion store  pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd inversion store  buf NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_inversion_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_inversion_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_inversion_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd inversion show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd inversion show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd inversion show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd inversion show pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd inversion show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_inversion_show != NULL)
		ret = pdata->lcd_inversion_show(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_lcd_scan_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd scan store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd scan store  fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd scan store  dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd scan store  pdata NULL Pointer!\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd scan store  buf NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}
	if ((saved_command_line != NULL) && (strstr(saved_command_line,
		"androidboot.swtype=factory") != NULL)) {
		if (pdata->lcd_scan_store != NULL) {
			dpufb_activate_vsync(dpufd);
			ret = pdata->lcd_scan_store(dpufd->pdev, buf, count);
			dpufb_deactivate_vsync(dpufd);
		}
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd scan store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_scan_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd scan show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd_scan_show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd_scan_show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd_scan_show pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd_scan_show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_scan_show != NULL)
		ret = pdata->lcd_scan_show(dpufd->pdev, buf);

	return ret;
}
static ssize_t dpufb_lcd_hbm_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd hbm ctrl show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd hbm ctrl show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd hbm ctrl show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd hbm ctrl show pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd hbm ctrl show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->lcd_hbm_ctrl_show != NULL)
		ret = pdata->lcd_hbm_ctrl_show(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_lcd_hbm_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd hbm ctrl store dev NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd hbm ctrl store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd hbm ctrl store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("lcd hbm ctrl store pdata NULL Pointer!\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd hbm ctrl store buf NULL Pointer!\n");
		return ret;
	}

	if (pdata->lcd_hbm_ctrl_store != NULL)
		ret = pdata->lcd_hbm_ctrl_store(dpufd->pdev, buf, count);

	if (ret < 0)
		DPU_FB_INFO("lcd hbm ctrl store ret is %d\n", ret);

	return count;
}

static ssize_t dpufb_lcd_amoled_vr_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd amoled vr mode show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd amoled vr mode show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd amoled vr mode show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd amoled vr mode show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_INFO("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_amoled_vr_mode_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_amoled_vr_mode_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_amoled_vr_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd amoled vr mode store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd amoled vr mode store buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd amoled vr mode store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd amoled vr mode store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_amoled_vr_mode_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_amoled_vr_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}


static ssize_t dpufb_lcd_acl_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd acl ctrl show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd acl ctrl show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd acl ctrl show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd acl ctrl show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->lcd_acl_ctrl_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_acl_ctrl_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_acl_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd acl ctrl store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd acl ctrl store buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd acl ctrl store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd acl ctrl store dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}

	if (pdata->lcd_acl_ctrl_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_acl_ctrl_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}


static ssize_t dpufb_amoled_pcd_errflag_check(struct device *dev,
	 struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("amoled pcd errflag check dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("amoled pcd errflag check fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("amoled pcd errflag check dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("amoled pcd errflag check pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("amoled pcd errflag check buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->amoled_pcd_errflag_check != NULL)
		ret = pdata->amoled_pcd_errflag_check(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_panel_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("panel info show dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("panel info show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("panel info show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("panel info show pdata NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("panel info show buf NULL Pointer!\n");
		return -1;
	}

	if (pdata->panel_info_show != NULL)
		ret = pdata->panel_info_show(dpufd->pdev, buf);

	return ret;
}

static ssize_t dpufb_lcd_acm_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd acm state show dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd acm state show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd acm state show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd acm state show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_acm_state_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_acm_state_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_acm_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd acm state store dev NULL Pointer!\n");
		return ret;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd acm state store buf NULL Pointer!\n");
		return ret;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd acm state store fbi NULL Pointer!\n");
		return ret;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd acm state store dpufd NULL Pointer!\n");
		return ret;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return ret;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_acm_state_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_acm_state_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	if (ret < 0)
		DPU_FB_INFO("lcd acm state store ret is %d\n", ret);

	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_lcd_gmp_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret = -1;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd gmp state showN dev ULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd gmp state show buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd gmp state show fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("lcd gmp state show dpufd NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_gmp_state_show != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_gmp_state_show(dpufd->pdev, buf);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

static ssize_t dpufb_lcd_gmp_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("lcd gmp state store dev NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("lcd gmp state store buf NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("lcd gmp state store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		ret = -EINVAL;
		goto err_out;
	}
	if (pdata->lcd_gmp_state_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->lcd_gmp_state_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return count;
}

static ssize_t dpufb_gamma_dynamic_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("gamma dynamic store dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("gamma dynamic store fbi NULL Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("gamma dynamic store dpufd NULL Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("gamma dynamic store bud NULL Pointer!\n");
		return -1;
	}

	return count;
}

static ssize_t hisi_alpm_function_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
#if defined(CONFIG_HISI_FB_AOD) || defined(CONFIG_DPU_FB_AP_AOD)
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	ssize_t ret;

	if (dev == NULL) {
		DPU_FB_ERR("alpm function store NULL dev Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("alpm function store NULL buf Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("alpm function store NULL fbi Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("alpm function storeNULL dpufd Pointer!\n");
		return -1;
	}
	if (strlen(buf) >= max_buf) {
		DPU_FB_ERR("buf overflow!\n");
		return -1;
	}

	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		return -1;
	}

	ret = sscanf(buf, "%u", &dpufd->aod_function);
	if (!ret) {
		DPU_FB_ERR("sscanf return invaild:%zd\n", ret);
		return -1;
	}
#endif
	return count;
}

static ssize_t hisi_alpm_function_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
#if defined(CONFIG_HISI_FB_AOD) || defined(CONFIG_DPU_FB_AP_AOD)
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (dev == NULL) {
		DPU_FB_ERR("alpm function show NULL dev Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("alpm function show NULL buf Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("alpm function show NULL fbi Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("alpm function show NULL dpufd Pointer!\n");
		return -1;
	}

	ret = snprintf(buf, PAGE_SIZE, "aod_function = %d\n",
			dpufd->aod_function);
#else
	ret = snprintf(buf, PAGE_SIZE, "aod_function = %d\n", 0);
#endif
	return ret;
}

static ssize_t hisi_alpm_setting_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	ssize_t ret;

	if (dev == NULL) {
		DPU_FB_ERR("alpm setting store NULL dev Pointer!\n");
		return -1;
	}

	if (buf == NULL) {
		DPU_FB_ERR("alpm setting store NULL buf Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (fbi == NULL) {
		DPU_FB_ERR("alpm setting store NULL fbi Pointer!\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)fbi->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("alpm setting store NULL dpufd Pointer!\n");
		return -1;
	}

	if (strlen(buf) >= max_buf) {
		DPU_FB_ERR("buf overflow!\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("NULL pdata Pointer!\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	pinfo = &(dpufd->panel_info);
	if (!dpufd->panel_power_on) {
		DPU_FB_ERR("fb%d, panel power off!\n", dpufd->index);
		goto err_out;
	}

	if (pdata->amoled_alpm_setting_store != NULL) {
		dpufb_activate_vsync(dpufd);
		ret = pdata->amoled_alpm_setting_store(dpufd->pdev,
			buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);
	return count;
}

/*lint -e665, -e514, -e84, -e866, -e886, -e846, -e778*/
static DEVICE_ATTR(frame_update, S_IRUGO|S_IWUSR,
	dpufb_frame_update_show, dpufb_frame_update_store);
static DEVICE_ATTR(lcd_model, 0644, dpufb_lcd_model_show, NULL);
static DEVICE_ATTR(lcd_cabc_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_cabc_mode_show, dpufb_lcd_cabc_mode_store);
static DEVICE_ATTR(lcd_ce_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_ce_mode_show, dpufb_lcd_ce_mode_store);
static DEVICE_ATTR(check_lcd_status, S_IRUGO,
	dpufb_lcd_check_reg_show, NULL);
static DEVICE_ATTR(lcd_mipi_detect, S_IRUGO,
	dpufb_lcd_mipi_detect_show, NULL);
static DEVICE_ATTR(frame_count, S_IRUGO, dpufb_frame_count_show, NULL);
static DEVICE_ATTR(mipi_dsi_bit_clk_upt, S_IRUGO|S_IWUSR,
	dpufb_mipi_dsi_bit_clk_upt_show, dpufb_mipi_dsi_bit_clk_upt_store);
static DEVICE_ATTR(panel_mode_switch, S_IRUGO|S_IWUSR, NULL,
	dpufb_panel_mode_switch_store);
static DEVICE_ATTR(lcd_hkadc, S_IRUGO|S_IWUSR, dpufb_lcd_hkadc_debug_show,
	dpufb_lcd_hkadc_debug_store);
static DEVICE_ATTR(lcd_checksum, S_IRUGO|S_IWUSR,
	dpufb_lcd_gram_check_show, dpufb_lcd_gram_check_store);
static DEVICE_ATTR(lcd_dynamic_checksum, S_IRUGO|S_IWUSR,
	dpufb_lcd_dynamic_sram_check_show,
		dpufb_lcd_dynamic_sram_check_store);
static DEVICE_ATTR(lcd_color_temperature, S_IRUGO|S_IWUSR,
	dpufb_lcd_color_temperature_show, dpufb_lcd_color_temperature_store);
static DEVICE_ATTR(lcd_ic_color_enhancement_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_ic_color_enhancement_mode_show,
		dpufb_lcd_ic_color_enhancement_mode_store);
static DEVICE_ATTR(led_rg_lcd_color_temperature, S_IRUGO|S_IWUSR,
	dpufb_led_rg_lcd_color_temperature_show,
		dpufb_led_rg_lcd_color_temperature_store);
static DEVICE_ATTR(lcd_comform_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_comform_mode_show, dpufb_lcd_comform_mode_store);
static DEVICE_ATTR(lcd_cinema_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_cinema_mode_show, dpufb_lcd_cinema_mode_store);
static DEVICE_ATTR(lcd_support_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_support_mode_show, dpufb_lcd_support_mode_store);
static DEVICE_ATTR(lcd_voltage_enable, S_IWUSR, NULL,
	dpufb_lcd_voltage_enable_store);
static DEVICE_ATTR(lcd_bist_check, S_IRUSR|S_IRGRP,
	dpufb_lcd_bist_check, NULL);
static DEVICE_ATTR(lcd_func_switch, S_IRUGO|S_IWUSR,
	dpufb_lcd_func_switch_show, dpufb_lcd_func_switch_store);
static DEVICE_ATTR(lcd_sleep_ctrl, S_IRUGO|S_IWUSR,
	dpufb_lcd_sleep_ctrl_show, dpufb_lcd_sleep_ctrl_store);
static DEVICE_ATTR(effect_al, S_IRUGO|S_IWUSR,
	dpufb_effect_al_show, dpufb_effect_al_store);
static DEVICE_ATTR(effect_ce, S_IRUGO|S_IWUSR,
	dpufb_effect_ce_show, dpufb_effect_ce_store);
static DEVICE_ATTR(effect_hdr_mode, S_IRUGO|S_IWUSR,
	dpufb_effect_hdr_mode_show, dpufb_effect_hdr_mode_store); //lint !e866
static DEVICE_ATTR(effect_bl, S_IRUGO, dpufb_effect_bl_show, NULL);
static DEVICE_ATTR(effect_bl_enable, S_IRUGO|S_IWUSR,
	dpufb_effect_bl_enable_show, dpufb_effect_bl_enable_store);
static DEVICE_ATTR(effect_sre, S_IRUGO|S_IWUSR,
	dpufb_effect_sre_show, dpufb_effect_sre_store);
static DEVICE_ATTR(effect_metadata, S_IRUGO|S_IWUSR,
	dpufb_effect_metadata_show, dpufb_effect_metadata_store);
static DEVICE_ATTR(effect_available, S_IRUGO,
	dpufb_effect_available_show, NULL);
static DEVICE_ATTR(lcd_test_config, 0640, dpufb_lcd_test_config_show,
	dpufb_lcd_test_config_store);
static DEVICE_ATTR(lcd_reg_read, 0600, dpufb_lcd_reg_read_show,
	dpufb_lcd_reg_read_store);
static DEVICE_ATTR(lcd_support_checkmode, S_IRUGO|S_IWUSR,
	dpufb_lcd_support_checkmode_show, NULL);
static DEVICE_ATTR(lcd_lp2hs_mipi_check, S_IRUGO|S_IWUSR,
	dpufb_lcd_lp2hs_mipi_check_show, dpufb_lcd_lp2hs_mipi_check_store);
static DEVICE_ATTR(lcd_inversion_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_inversion_show, dpufb_lcd_inversion_store);
static DEVICE_ATTR(lcd_scan_mode, S_IRUGO|S_IWUSR,
	dpufb_lcd_scan_show, dpufb_lcd_scan_store);
static DEVICE_ATTR(amoled_pcd_errflag_check, 0644,
	dpufb_amoled_pcd_errflag_check, NULL);
static DEVICE_ATTR(amoled_hbm, S_IRUGO|S_IWUSR,
	dpufb_lcd_hbm_ctrl_show, dpufb_lcd_hbm_ctrl_store);
static DEVICE_ATTR(gamma_dynamic, S_IRUGO|S_IWUSR, NULL,
	dpufb_gamma_dynamic_store);
static DEVICE_ATTR(panel_info, 0644, dpufb_panel_info_show, NULL);
static DEVICE_ATTR(lcd_acm_state, S_IRUGO|S_IWUSR,
	dpufb_lcd_acm_state_show, dpufb_lcd_acm_state_store);
static DEVICE_ATTR(amoled_acl, S_IRUGO|S_IWUSR,
	dpufb_lcd_acl_ctrl_show, dpufb_lcd_acl_ctrl_store);
static DEVICE_ATTR(lcd_gmp_state, S_IRUGO|S_IWUSR,
	dpufb_lcd_gmp_state_show, dpufb_lcd_gmp_state_store);
static DEVICE_ATTR(amoled_vr_mode, 0644, dpufb_lcd_amoled_vr_mode_show,
	dpufb_lcd_amoled_vr_mode_store);
static DEVICE_ATTR(lcd_fps_scence, (S_IRUGO|S_IWUSR),
	dpufb_lcd_fps_scence_show, dpufb_lcd_fps_scence_store);
static DEVICE_ATTR(alpm_function, 0644, hisi_alpm_function_show,
	hisi_alpm_function_store);
static DEVICE_ATTR(alpm_setting, 0644, NULL, hisi_alpm_setting_store);
/*lint +e665, +e514, +e84, +e866, +e886, +e846, +e778*/

void dpufb_sysfs_attrs_add(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (dpufd->sysfs_attrs_append_fnc != NULL) {
#if !defined(CONFIG_LCDKIT_DRIVER) && !defined(CONFIG_LCD_KIT_DRIVER)
			dpufd->sysfs_attrs_append_fnc(dpufd,
				&dev_attr_panel_mode_switch.attr);
#endif
			dpufd->sysfs_attrs_append_fnc(dpufd,
				&dev_attr_check_lcd_status.attr);
		}
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);
}
/*lint +e559*/
#pragma GCC diagnostic pop
