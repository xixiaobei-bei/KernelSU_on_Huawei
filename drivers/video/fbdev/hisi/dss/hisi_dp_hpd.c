/* Copyright (c) 2010-2015, Hisilicon Tech. Co., Ltd. All rights reserved.
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

int hisi_dp_hpd_register(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	struct dp_ctrl *dptx = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[DP] dpufd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(dpufd->dp);
	if (dpufd->pdev == NULL)
		return -EINVAL;

	dptx->hpd_state = HPD_OFF;

	return ret;
}

void hisi_dp_hpd_unregister(struct dpu_fb_data_type *dpufd)
{
	struct dp_ctrl *dptx = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[DP] dpufd is NULL!\n");
		return;
	}

	if (dpufd->pdev == NULL)
		return;

	dptx = &(dpufd->dp);
	if (dptx == NULL) {
		dev_err(&dpufd->pdev->dev, "invalid dptx!\n");
		return;
	}

	dptx->hpd_state = HPD_OFF;
}
