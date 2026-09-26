/*
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <drm/drmP.h>
#include "hisi_drm_drv.h"
#include "drm_crtc.h"
#include "drm_crtc_helper.h"
#include "hisi_defs.h"

struct hisi_framebuffer {
	struct drm_framebuffer base;
};
#define to_hisi_framebuffer(x) container_of(x, struct hisi_framebuffer, base)

static int hisi_framebuffer_create_handle(
	struct drm_framebuffer *fb,
	struct drm_file *file_priv,
	unsigned int *handle)
{
	HISI_DRM_INFO("+");

	UNUSED(fb);
	UNUSED(file_priv);
	UNUSED(handle);

	HISI_DRM_INFO("-");

	return 0;
}

static void hisi_framebuffer_destroy(struct drm_framebuffer *fb)
{
	struct hisi_framebuffer *hisi_fb = NULL;

	HISI_DRM_INFO("+");

	if (!fb) {
		HISI_DRM_ERR("fb is nullptr!");
		return;
	}

	hisi_fb = to_hisi_framebuffer(fb);

	drm_framebuffer_cleanup(fb);

	kfree(hisi_fb);
	hisi_fb = NULL;

	HISI_DRM_INFO("-");
}

static int hisi_framebuffer_dirty(struct drm_framebuffer *fb,
		struct drm_file *file_priv, uint32_t flags, uint32_t color,
		struct drm_clip_rect *clips, uint32_t num_clips)
{
	struct drm_device *drm_dev = NULL;
	struct hisi_drm_private *hisi_priv = NULL;
	struct drm_fb_helper *fb_helper = NULL;
	struct fb_info *info = NULL;
	struct fb_var_screeninfo var;
	int ret = 0;

	HISI_DRM_DEBUG("+");

	drm_check_and_return((fb == NULL), -EINVAL, ERR, "fb is nullptr!");
	drm_check_and_return((file_priv == NULL), -EINVAL, DEBUG, "file_priv is nullptr");

	drm_dev = fb->dev;
	drm_check_and_return((drm_dev == NULL), -EINVAL, ERR, "drm_dev is nullptr!");

	hisi_priv = drm_dev->dev_private;
	drm_check_and_return((hisi_priv == NULL), -EINVAL, ERR, "hisi_priv is nullptr!");

	fb_helper = hisi_priv->fb_helper;
	drm_check_and_return((fb_helper == NULL), -EINVAL, ERR, "fb_helper is nullptr!");

	info = fb_helper->fbdev;
	drm_check_and_return((info == NULL), -EINVAL, ERR, "info is nullptr!");

	mutex_lock(&hisi_priv->suspend_mutex);
	if (hisi_priv->suspend) {
		HISI_DRM_INFO("in suspend status, return!");
		mutex_unlock(&hisi_priv->suspend_mutex);
		return ret;
	}

	var.xoffset = 0;
	var.yoffset = 0;

	ret = drm_fb_helper_pan_display(&var, info);
	if (ret)
		HISI_DRM_ERR("failed to pan_display! ret=%d!", ret);

	mutex_unlock(&hisi_priv->suspend_mutex);

	HISI_DRM_INFO("-");

	return ret;
}


static const struct drm_framebuffer_funcs hisi_framebuffer_funcs = {
	.create_handle = hisi_framebuffer_create_handle,
	.destroy = hisi_framebuffer_destroy,
	.dirty = hisi_framebuffer_dirty,
};

struct drm_framebuffer *hisi_framebuffer_init(struct drm_device *dev,
	struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct hisi_framebuffer *hisi_fb = NULL;
	struct drm_framebuffer *fb = NULL;
	int ret;

	HISI_DRM_INFO("+");

	if (!dev) {
		HISI_DRM_ERR("dev is nullptr!");
		return NULL;
	}

	if (!mode_cmd) {
		HISI_DRM_ERR("mode_cmd is nullptr!");
		return NULL;
	}

	hisi_fb = kzalloc(sizeof(*hisi_fb), GFP_KERNEL);
	if (!hisi_fb) {
		HISI_DRM_ERR("hisi_fb is nullptr!");
		return NULL;
	}

	fb = &hisi_fb->base;
	if (!fb) {
		HISI_DRM_ERR("fb is nullptr!");
		goto err_fb;
	}

	drm_helper_mode_fill_fb_struct(dev, fb, mode_cmd);

	ret = drm_framebuffer_init(dev, fb, &hisi_framebuffer_funcs);
	if (ret) {
		HISI_DRM_ERR("failed to init framebuffer! ret=%d!", ret);
		goto err_drm_framebuffer_init;
	}

	HISI_DRM_INFO("-");

	return fb;

err_drm_framebuffer_init:

err_fb:
	kfree(hisi_fb);
	hisi_fb = NULL;

	return NULL;
}
