/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/fb.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

#include "dkmd_connector.h"
#include "dkmd_utils.h"
#include "dpu_fb.h"
#include "dpu_comp.h"
#include "dkmd_dpu.h"

#define DEVICE_FB_MAX_COUNT 32
static struct device_fb* g_device_fb[DEVICE_FB_MAX_COUNT];

static bool dpu_fb_enable_fastboot_display(struct device_fb *dpu_fb)
{
	// TODO: init dpu isr, etc.

	return true;
}

static bool is_valid_next_composer(struct composer *comp)
{
	if (!comp)
		return false;

	if (!comp->on)
		return false;

	if (!comp->off)
		return false;

	if (!comp->present)
		return false;

	return true;
}

static void dpu_fb_blank_power_on(struct device_fb *dpu_fb, struct composer *comp)
{
	int ret;

	if (dpu_fb->power_on)
		return;

	if (!comp->on)
		return;

	ret = comp->on(comp->pdev);
	if (ret) {
		dpu_pr_err("next composer power on fail");
		dpu_fb->power_on = false;
		return;
	}

	dpu_fb->power_on = true;
}

static void dpu_fb_blank_power_off(struct device_fb *dpu_fb, struct composer *comp)
{
	int ret;

	if (!dpu_fb->power_on)
		return;

	if (!comp->off)
		return;

	ret = comp->off(comp->pdev);
	if (ret) {
		dpu_pr_err("next composer power on fail");
		dpu_fb->power_on = true;
		return;
	}

	dpu_fb->power_on = false;
}

static int dpu_fb_blank_sub(int blank_mode, struct device_fb *dpu_fb)
{
	struct composer *comp = dpu_fb->next_composer;

	dpu_pr_info("fb-%d enter ++++, power_on=%d", dpu_fb->scene_id, dpu_fb->power_on);

	if (!is_valid_next_composer(comp)) {
		dpu_pr_err("next_composer is null");
		return -1;
	}

	/* TODO: other modules listen to the blank event, FB_EARLY_EVENT_BLANK or FB_EVENT_BLANK
	 * such as, AOD. those event had been sended at fbmen.c
	 */
	down(&dpu_fb->blank_sem);
	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
		dpu_fb_blank_power_on(dpu_fb, comp);
		break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_NORMAL:
	case FB_BLANK_POWERDOWN:
	default:
		dpu_fb_blank_power_off(dpu_fb, comp);
		break;
	}
	up(&dpu_fb->blank_sem);

	dpu_pr_info("fb-%d exit ----, power_on=%d", dpu_fb->scene_id, dpu_fb->power_on);

	return 0;
}

static int dpu_fb_create_fence(struct device_fb *dpu_fb, void __user *argp)
{
	struct composer *comp = dpu_fb->next_composer;
	int fence_fd = -1;

	if (!is_valid_next_composer(comp))
		return -1;

	fence_fd = comp->create_fence(comp->pdev);

	if (copy_to_user(argp, &fence_fd, sizeof(fence_fd)) != 0) {
		dpu_pr_err("copy fence to user fail, fence_fd = %d", fence_fd);
		return -1;
	}

	return 0;
}

static int dpu_fb_present(struct device_fb *dpu_fb, void __user *argp)
{
	struct disp_frame frame;
	struct composer *comp = dpu_fb->next_composer;
	int ret = 0;

	if (!is_valid_next_composer(comp))
		return -1;

	down(&dpu_fb->blank_sem);
	if (!dpu_fb->power_on) {
		goto present_fail;
	}

	if (copy_from_user(&frame, argp, sizeof(frame)) != 0) {
		dpu_pr_err("copy from user frame info fail");
		ret = -1;
		goto present_fail;
	}

	ret = comp->present(comp->pdev, &frame);
	if (ret) {
		dpu_pr_err("fb %u present fail", dpu_fb->scene_id);
		goto present_fail;
	}

present_fail:
	up(&dpu_fb->blank_sem);
	return ret;
}

// ============================================================================

static struct fb_fix_var_screeninfo g_fb_fix_var_screeninfo[FB_FOARMT_MAX] = {
	 /* FB_FORMAT_BGRA8888, */
	{ FB_TYPE_PACKED_PIXELS, 1, 1, FB_VMODE_NONINTERLACED, 0, 8, 16, 24, 8, 8, 8, 8, 0, 0, 0, 0, 4 },

	/* FB_FORMAT_RGB565 */
	{FB_TYPE_PACKED_PIXELS, 1, 1, FB_VMODE_NONINTERLACED, 0, 5, 11, 0, 5, 6, 5, 0, 0, 0, 0, 0, 2},

	/* FB_FORMAT_YUV_422_I */
	{FB_TYPE_INTERLEAVED_PLANES, 2, 1, FB_VMODE_NONINTERLACED, 0, 5, 11, 0, 5, 6, 5, 0, 0, 0, 0, 0, 2},
};

static int dpu_fb_open(struct fb_info *info, int user)
{
	struct device_fb *dpu_fb = NULL;
	int ret = 0;

	dpu_fb = (struct device_fb *)info->par;

	if (!dpu_fb->be_connected) {
		dpu_pr_info("fb %d is not connected", dpu_fb->scene_id);
		return 0;
	}

	/* fb have been opened */
	if (atomic_read(&dpu_fb->ref_cnt) > 0)
		return 0;

	/* if enable fastboot display succ, skip blank
	 * else, need unblank dpu
	 */
	if (!dpu_fb_enable_fastboot_display(dpu_fb))
		ret = dpu_fb_blank_sub(FB_BLANK_UNBLANK, dpu_fb);

	atomic_inc(&dpu_fb->ref_cnt);

	return ret;
}

static int dpu_fb_release(struct fb_info *info, int user)
{
	struct device_fb *dpu_fb = NULL;
	int ret = 0;

	dpu_fb = (struct device_fb *)info->par;

	if (!dpu_fb->be_connected) {
		dpu_pr_info("fb %d is not connected", dpu_fb->scene_id);
		return 0;
	}

	/* fb have not been opened, return */
	if (atomic_read(&dpu_fb->ref_cnt) == 0)
		return 0;

	/* ref_cnt is not 0 */
	if (!atomic_sub_and_test(1, &dpu_fb->ref_cnt))
		return 0;

	ret = dpu_fb_blank_sub(FB_BLANK_POWERDOWN, dpu_fb);
	if (ret)
		dpu_pr_info("dpu_fb %u, blank pown down error, ret=%d",
			dpu_fb->scene_id, ret);

	return ret;
}

static int dpu_fb_blank(int blank_mode, struct fb_info *info)
{
	struct device_fb *dpu_fb = NULL;
	int ret = 0;

	dpu_fb = (struct device_fb *)info->par;

	/* TODO:
	 * other modules listen to the blank event, FB_EARLY_EVENT_BLANK or FB_EVENT_BLANK
	 * such as, AOD. those event had been sended at fbmen.c
	 */
	ret = dpu_fb_blank_sub(blank_mode, dpu_fb);
	if (ret)
		dpu_pr_info("blank sub fail");

	return ret;
}

static int dpu_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	struct device_fb *dpu_fb = NULL;
	void __user *argp = (void __user *)(uintptr_t)arg;
	int ret = 0;

	dpu_fb = (struct device_fb *)info->par;

	switch (cmd) {
	case DISP_CREATE_FENCE:
		return dpu_fb_create_fence(dpu_fb, argp);
	case DISP_PRESENT:
		return dpu_fb_present(dpu_fb, argp);
	default:
		dpu_pr_err("fb-%d ioctl fail unsupport cmd, cmd = 0x%x", dpu_fb->scene_id, cmd);
		return -1;
	}

	return ret;
}

static void fb_init_fbi_fix_info(struct device_fb *fb,
	struct fb_fix_var_screeninfo *screen_info, struct fb_fix_screeninfo *fix)
{
	const struct dkmd_connector_info *connector_info = fb->connector_info;

	fb->bpp = screen_info->bpp;
	fix->type_aux = 0;
	fix->visual = FB_VISUAL_TRUECOLOR;
	fix->ywrapstep = 0;
	fix->mmio_start = 0;
	fix->mmio_len = 0;
	fix->accel = FB_ACCEL_NONE;

	fix->type = screen_info->fix_type;
	fix->xpanstep = screen_info->fix_xpanstep;
	fix->ypanstep = screen_info->fix_ypanstep;

	snprintf(fix->id, sizeof(fix->id), "dpufb%d", fb->scene_id);

	fix->line_length = roundup(connector_info->xres * (uint32_t)fb->bpp, DMA_STRIDE_ALIGN);
	fix->smem_len = roundup(fix->line_length * connector_info->yres * FB_BUFFER_MAX_COUNT, PAGE_SIZE);
	fix->smem_start = 0;
	fix->reserved[0] = (uint16_t)is_mipi_cmd_panel(fb->connector_info);
}

static void fb_init_fbi_var_info(struct device_fb *fb,
	struct fb_fix_var_screeninfo *screen_info, struct fb_var_screeninfo *var)
{
	const struct dkmd_connector_info *connector_info = fb->connector_info;

	var->xoffset = 0;
	var->yoffset = 0;
	var->grayscale = 0;
	var->nonstd = 0;
	var->activate = FB_ACTIVATE_VBL;
	var->accel_flags = 0;
	var->sync = 0;
	var->rotate = 0;

	var->vmode = screen_info->var_vmode;
	var->blue.offset = screen_info->var_blue_offset;
	var->green.offset = screen_info->var_green_offset;
	var->red.offset = screen_info->var_red_offset;
	var->transp.offset = screen_info->var_transp_offset;
	var->blue.length = screen_info->var_blue_length;
	var->green.length = screen_info->var_green_length;
	var->red.length = screen_info->var_red_length;
	var->transp.length = screen_info->var_transp_length;
	var->blue.msb_right = screen_info->var_blue_msb_right;
	var->green.msb_right = screen_info->var_green_msb_right;
	var->red.msb_right = screen_info->var_red_msb_right;
	var->transp.msb_right = screen_info->var_transp_msb_right;

	var->xres = connector_info->xres;
	var->yres = connector_info->yres;
	var->height = connector_info->height;
	var->width = connector_info->width;
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres * FB_BUFFER_MAX_COUNT;
	var->bits_per_pixel = fb->bpp * 8;

	var->reserved[0] = connector_info->fps;
}

static struct fb_ops g_fb_ops = {
	.owner = THIS_MODULE,
	.fb_open = dpu_fb_open,
	.fb_release = dpu_fb_release,
	.fb_read = NULL,
	.fb_write = NULL,
	.fb_cursor = NULL,
	.fb_check_var = NULL,
	.fb_set_par = NULL,
	.fb_setcolreg = NULL,
	.fb_blank = dpu_fb_blank,
	.fb_pan_display = NULL,
	.fb_fillrect = NULL,
	.fb_copyarea = NULL,
	.fb_imageblit = NULL,
	.fb_sync = NULL,
	.fb_ioctl = dpu_fb_ioctl,
	.fb_compat_ioctl = dpu_fb_ioctl,
	.fb_mmap = NULL,
};

void fb_device_register(struct composer *comp)
{
	struct device_fb *fb = NULL;
	struct fb_info *fbi = NULL;

	if (!comp) {
		dpu_pr_err("comp is null!");
		return;
	}

	if (comp->scene_id >= DEVICE_FB_MAX_COUNT)
		return;

	fb = kzalloc(sizeof(*fb), GFP_KERNEL);
	if (!fb) {
		dpu_pr_err("alloc fb device fail");
		return;
	}
	fb->scene_id = comp->scene_id;
	fb->next_composer = comp;
	fb->connector_info = comp->connector_info;
	sema_init(&fb->blank_sem, 1);

	fbi = framebuffer_alloc(0, NULL);
	if (!fbi) {
		dpu_pr_err("alloc fbi fail");
		kfree(fb);
		return;
	}

	fb_init_fbi_fix_info(fb, &g_fb_fix_var_screeninfo[FB_FORMAT_BGRA8888], &fbi->fix);
	fb_init_fbi_var_info(fb, &g_fb_fix_var_screeninfo[FB_FORMAT_BGRA8888], &fbi->var);
	fbi->par = fb;
	fbi->screen_base = 0;
	fbi->fbops = &g_fb_ops;
	fbi->flags = FBINFO_FLAG_DEFAULT;
	fbi->pseudo_palette = NULL;

	if (register_framebuffer(fbi) < 0) {
		dpu_pr_err("scene-%d failed to register_framebuffer!", comp->scene_id);
		kfree(fb);
		return;
	}

	if (!is_dp_panel(fb->connector_info))
		fb->be_connected = true;

	fb->fbi_info = fbi;
	g_device_fb[comp->scene_id] = fb;

	dpu_pr_info("FrameBuffer[%d] %dx%d size=%d bytes is registered successfully!\n",
		fb->scene_id, fbi->var.xres, fbi->var.yres, fbi->fix.smem_len);
}

void fb_device_unregister(struct composer *comp)
{
	struct device_fb *fb = NULL;

	if (!comp) {
		dpu_pr_err("comp is null!");
		return;
	}

	fb = g_device_fb[comp->scene_id];
	if (!fb) {
		dpu_pr_err("fb is null!");
		return;
	}

	unregister_framebuffer(fb->fbi_info);
}