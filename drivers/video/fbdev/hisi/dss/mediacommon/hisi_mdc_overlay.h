/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_MDC_OVERLAY_H
#define HISI_MDC_OVERLAY_H

#define MDC_RCHN_V DSS_RCHN_V0
#define MDC_WCHN_W DSS_WCHN_W1
#define MDC_OVL DSS_OVL3

void hisi_mdc_preoverlay_async_play(struct kthread_work *work);
int hisi_ov_media_common_play(struct dpu_fb_data_type *dpufd, const void __user *argp);

#endif
