/* Copyright (c) 2010-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#include "hisi_dp.h"
#include <linux/device.h>

int hisi_dp_hpd_register(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	struct dp_ctrl *dptx = NULL;

	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	if (!hisifd->pdev) {
		return -EINVAL;
	}
	if (!dptx) {
		dev_err(&hisifd->pdev->dev, "invalid dptx!\n");
		return -EINVAL;
	}

	//if (hisifd->sysfs_attrs_append_fnc) {
	//	hisifd->sysfs_attrs_append_fnc(hisifd, &dev_attr_dp_hpd.attr);
	//}

	dptx->hpd_state = HPD_OFF;

	return ret;
}

void hisi_dp_hpd_unregister(struct hisi_fb_data_type *hisifd)
{
	struct dp_ctrl *dptx = NULL;

	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return;
	}

	if (!hisifd->pdev) {
		return;
	}
	dptx = &(hisifd->dp);
	if (!dptx) {
		dev_err(&hisifd->pdev->dev, "invalid dptx!\n");
		return;
	}

	dptx->hpd_state = HPD_OFF;
}
