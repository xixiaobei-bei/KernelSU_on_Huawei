/*
 * da_combine_compat.c
 *
 * codec controller
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO.Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hisi/da_combine/da_combine_compat.h>

#include <linux/errno.h>
#include <linux/hisi/da_combine/da_combine_utils.h>
#include <linux/hisi/audio_log.h>

#define LOG_TAG "DA_combine_compat"

struct compat_data {
	struct hi_cdc_ctrl *hi_cdc;
	struct da_combine_irq *irq;
};

static struct compat_data *g_compat_data;

int da_combine_compat_init(struct hi_cdc_ctrl *hi_cdc, struct da_combine_irq *irq)
{
	if (hi_cdc == NULL) {
		AUDIO_LOGE("hi cdc is null");
		return -EINVAL;
	}

	if (irq == NULL) {
		AUDIO_LOGE("irq is null");
		return -EINVAL;
	}

	g_compat_data = kzalloc(sizeof(*g_compat_data), GFP_KERNEL);
	if (g_compat_data == NULL) {
		AUDIO_LOGE("kzalloc error");
		return -ENOMEM;
	}

	g_compat_data->hi_cdc = hi_cdc;
	g_compat_data->irq = irq;

	AUDIO_LOGI("init ok");

	return 0;
}

void da_combine_compat_deinit(void)
{
	if (g_compat_data != NULL) {
		kfree(g_compat_data);
		g_compat_data = NULL;
	}
}

