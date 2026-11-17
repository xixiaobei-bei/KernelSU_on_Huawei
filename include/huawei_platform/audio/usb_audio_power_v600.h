/*
 * usb_audio_power_v600.h
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
#ifndef __USB_AUDIO_POWER_V600_H__
#define __USB_AUDIO_POWER_V600_H__

#define IOCTL_USB_AUDIO_POWER_BUCKBOOST_NO_HEADSET_CMD     _IO('Q', 0x01)
#define IOCTL_USB_AUDIO_POWER_SCHARGER_CMD                 _IO('Q', 0x02)
#ifdef CONFIG_USB_AUDIO_POWER_V600
void set_otg_switch_enable_v600(void);
void restore_headset_state(void);
#else
static inline void set_otg_switch_enable_v600(void)
{
}
static inline void restore_headset_state(void)
{
}
#endif
#endif // __USB_AUDIO_POWER_V600_H_
