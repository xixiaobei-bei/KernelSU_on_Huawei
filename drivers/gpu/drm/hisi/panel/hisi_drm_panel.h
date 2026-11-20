/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
/*lint -e553*/

#ifndef HISI_DRM_PANEL_H
#define HISI_DRM_PANEL_H

#include <linux/kernel.h>
#include <linux/backlight.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drmP.h>
#include <drm/drm_mode_object.h>
#include "../hisi_dpe.h"
#include "../hisi_drm_dpe_utils.h"
#include "hisi_drm_dsi.h"
#include "hisi_panel.h"

struct hisi_drm_panel {
	struct drm_panel base;
	struct mipi_dsi_device *dsi;
	struct backlight_device *backlight;
	struct regulator *supply;

	bool prepared;
	bool enabled;

	const struct drm_display_mode *mode;
	struct hisi_panel_info *panel_info;
	enum panel_status status;
};

int hisi_panel_add(struct hisi_drm_panel *panel, struct drm_panel_funcs *funcs);

static inline struct hisi_drm_panel *to_hisi_panel(struct drm_panel *panel)
{
	return container_of(panel, struct hisi_drm_panel, base);
}
#endif

/*lint +e553*/
