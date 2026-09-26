/* Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_fb.h"
#include "hisi_fb_panel.h"

int panel_next_bypass_powerdown_ulps_support(struct platform_device *pdev)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;
	struct dpu_fb_panel_data *next_pdata = NULL;
	struct platform_device *next_pdev = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return 0;
	}
	pdata = dev_get_platdata(&pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL");
		return 0;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(&next_pdev->dev);
		if ((next_pdata) &&
			(next_pdata->panel_bypass_powerdown_ulps_support))
			ret = next_pdata->panel_bypass_powerdown_ulps_support(
					next_pdev);
	}
	return ret;
}


