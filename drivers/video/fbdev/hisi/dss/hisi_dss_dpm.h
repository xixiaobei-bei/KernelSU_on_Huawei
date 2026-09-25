/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_DSS_DPM_H
#define HISI_DSS_DPM_H

#include "hisi_fb.h"

struct hisi_dss_dpm {
	bool dpm_inited;
	struct hrtimer dpm_hrtimer;
	struct workqueue_struct *dpm_check_wq;
	struct work_struct dpm_check_work;
	struct hisi_fb_data_type *hisifd;
};

#define DSS_DPM_CHECK_PERIOD 2000 /* ms */

void hisi_dss_dpm_init(struct hisi_fb_data_type *hisifd);
void hisi_dss_dpm_deinit(struct hisi_fb_data_type *hisifd);

#endif

