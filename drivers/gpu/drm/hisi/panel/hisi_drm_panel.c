/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */
#include "hisi_drm_panel.h"

int hisi_panel_add(struct hisi_drm_panel *panel, struct drm_panel_funcs *funcs)
{
	struct device *dev = NULL;
	int ret = 0;

	if (!panel) {
		HISI_DRM_ERR("panel is NULL!!");
		return ret;
	}
	dev = &panel->dsi->dev;

	drm_panel_init(&panel->base);
	panel->base.funcs = funcs;
	panel->base.dev = dev;

	ret = drm_panel_add(&panel->base);
	return ret;
}
