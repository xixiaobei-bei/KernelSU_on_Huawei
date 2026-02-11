/*
 * usb_analog_hs_fsa4476.h
 *
 * usb analog headset fsa4476 driver
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

#ifndef __USB_ANALOG_HS_FSA4476__
#define __USB_ANALOG_HS_FSA4476__

#include "huawei_platform/audio/usb_analog_hs_interface.h"
#include "huawei_platform/usb/hw_pd_dev.h"

extern void pd_dpm_send_event(enum pd_dpm_cable_event_type event);

#ifdef CONFIG_USB_ANALOG_HS_FSA4476
int usb_ana_hs_fsa4476_check_hs_pluged_in(void);
int usb_ana_hs_fsa4476_dev_register(struct usb_analog_hs_dev *dev,
	void *codec_data);
bool check_usb_analog_hs_fsa4476_support(void);
void usb_ana_hs_fsa4476_plug_in_out_handle(int hs_state);
void usb_ana_hs_fsa4476_mic_switch_change_state(void);
#else
static inline int usb_ana_hs_fsa4476_check_hs_pluged_in(void)
{
	return 0;
}

static inline int usb_ana_hs_fsa4476_dev_register(struct usb_analog_hs_dev *dev,
	void *codec_data)
{
	return 0;
}

static inline bool check_usb_analog_hs_fsa4476_support(void)
{
	return false;
}

static inline void usb_ana_hs_fsa4476_plug_in_out_handle(int hs_state)
{
}

static inline void usb_ana_hs_fsa4476_mic_switch_change_state(void)
{
}
#endif

#endif // USB_ANALOG_HS_FSA4476
