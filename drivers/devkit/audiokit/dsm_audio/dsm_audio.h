/*
 * dsm_audio.h
 *
 * dsm audio driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __DSM_AUDIO_H__
#define __DSM_AUDIO_H__

#include <dsm/dsm_pub.h>

#define DSM_AUDIO_BUF_SIZE   4096 // byte
#define DSM_AUDIO_NAME       "dsm_audio_info"

#define DSM_SMARTPA_BUF_SIZE 1024 // byte
#define DSM_SMARTPA_NAME     "dsm_smartpa"

#define DSM_ANC_HS_BUF_SIZE  1024 // byte
#define DSM_ANC_HS_NAME      "dsm_anc_hs"

enum audio_device_type {
	AUDIO_CODEC = 0,
	AUDIO_SMARTPA,
	AUDIO_ANC_HS,
	AUDIO_DEVICE_MAX
};

#ifdef CONFIG_HUAWEI_DSM_AUDIO
int audio_dsm_report_num(enum audio_device_type device_type, int error_no,
		unsigned int mesg_no);
int audio_dsm_report_info(enum audio_device_type device_type, int error_no,
		char *fmt, ...);
#else
static inline int audio_dsm_report_num(enum audio_device_type device_type,
		int error_no, unsigned int mesg_no)
{
	return 0;
}

static inline int audio_dsm_report_info(enum audio_device_type device_type,
		int error_no, char *fmt, ...)
{
	return 0;
}
#endif // !CONFIG_HUAWEI_DSM_AUDIO

#endif // __DSM_AUDIO_H__
