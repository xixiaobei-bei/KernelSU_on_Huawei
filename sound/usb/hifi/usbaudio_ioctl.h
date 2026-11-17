/*
 * usbaudio_ioctl.h
 *
 * usbaudio ioctl.
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#ifndef __USBAUDIO_IOCTL_H__
#define __USBAUDIO_IOCTL_H__

#define USBAUDIO_INFONAME_LEN 256
#define USBAUDIO_PCM_NUM      2
#define MAX_SAMPLE_RATE_NUM   7

enum usbaudio_stream_type {
	DOWNLINK_STREAM,
	UPLINK_STREAM,
	TYPE_MAX
};

enum usbaudio_stream_status {
	START_STREAM,
	STOP_STREAM,
	STATUS_MAX
};

enum usbaudio_controller_status {
	ACPU_CONTROL,
	DSP_CONTROL,
	CONTROL_MAX
};

enum usbaudio_dsp_reset_status {
	USBAUDIO_DSP_NORMAL,
	USBAUDIO_DSP_ABNORMAL,
};

struct usbaudio_info {
	unsigned int usbid;
	unsigned int dnlink_rate_table[MAX_SAMPLE_RATE_NUM];
	int controller_location;
	unsigned short uplink_channels;
	unsigned short dnlink_channels;
	unsigned char name[USBAUDIO_INFONAME_LEN];
	int sr_status;
};

void usbaudio_ctrl_query_info(struct usbaudio_info *usbinfo);
void usbaudio_ctrl_hifi_reset_inform(void);
int usbaudio_ctrl_usb_resume(void);
void usbaudio_set_nv_ready(void);

#endif /* __USBAUDIO_IOCTL_H__ */
