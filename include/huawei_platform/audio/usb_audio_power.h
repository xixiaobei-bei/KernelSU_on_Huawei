/*
 * usb_audio_power.h
 *
 * usb audio power control driver
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

#ifndef __USB_AUDIO_POWER_H__
#define __USB_AUDIO_POWER_H__

#define IOCTL_USB_AUDIO_POWER_BUCKBOOST_NO_HEADSET_CMD     _IO('Q', 0x01)
#define IOCTL_USB_AUDIO_POWER_SCHARGER_CMD                 _IO('Q', 0x02)

enum vboost_control_source_type {
	VBOOST_CONTROL_PM = 0,
	VBOOST_CONTROL_AUDIO,
};
enum mlib_voice_para_enum {
	AUDIO_POWER_GPIO_RESET = 0,
	AUDIO_POWER_GPIO_SET,
};

#ifdef CONFIG_USB_AUDIO_POWER
int bst_ctrl_enable(bool enable, enum vboost_control_source_type type);
int usb_audio_power_buckboost(void);
int usb_audio_power_scharger(void);
int usb_headset_plug_in(void);
int usb_headset_plug_out(void);
#else
static inline int bst_ctrl_enable(bool enable,
	enum vboost_control_source_type type)
{
	return 0;
}

static inline int usb_audio_power_buckboost(void)
{
	return 0;
}

static inline int usb_audio_power_scharger(void)
{
	return 0;
}

static inline int usb_headset_plug_in(void)
{
	return 0;
}

static inline int usb_headset_plug_out(void)
{
	return 0;
}

#endif

#endif // __USB_AUDIO_POWER_H__
