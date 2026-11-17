/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include "slimbus_da_combine.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <dsm/dsm_pub.h>

#include "linux/hisi/audio_log.h"
#include "slimbus_drv.h"
#include "slimbus.h"
#include "csmi.h"


/*lint -e750 -e730 -e785 -e574*/
#define LOG_TAG "slimbus_DA_combine"

void slimbus_da_combine_set_para_pr(enum slimbus_presence_rate *pr_table,
		uint32_t track_type, const struct slimbus_track_param *params)
{
	if (pr_table == NULL) {
		AUDIO_LOGE("input pr_table is null");
		return;
	}

	if (params) {
		if (params->rate == SLIMBUS_SAMPLE_RATE_8K)
			pr_table[track_type] = SLIMBUS_PR_8K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_16K)
			pr_table[track_type] = SLIMBUS_PR_16K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_32K)
			pr_table[track_type] = SLIMBUS_PR_32K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_48K)
			pr_table[track_type] = SLIMBUS_PR_48K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_96K)
			pr_table[track_type] = SLIMBUS_PR_96K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_192K)
			pr_table[track_type] = SLIMBUS_PR_192K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_768K)
			pr_table[track_type] = SLIMBUS_PR_768K;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_44K1)
			pr_table[track_type] = SLIMBUS_PR_44100;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_88K2)
			pr_table[track_type] = SLIMBUS_PR_88200;
		else if (params->rate == SLIMBUS_SAMPLE_RATE_176K4)
			pr_table[track_type] = SLIMBUS_PR_176400;
		else
			AUDIO_LOGE("sample rate is invalid: %d", params->rate);
	}
}

void slimbus_da_combine_release_device(struct slimbus_device_info *device)
{
	if (device == NULL) {
		AUDIO_LOGE("device is null");
		return;
	}

	if (device->slimbus_da_combine_para != NULL) {
		kfree(device->slimbus_da_combine_para);
		device->slimbus_da_combine_para = NULL;
	}

	mutex_destroy(&(device->rw_mutex));
	mutex_destroy(&(device->track_mutex));

	kfree(device);
	device = NULL;
}

