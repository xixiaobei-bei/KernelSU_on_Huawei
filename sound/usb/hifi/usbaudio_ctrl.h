/*
 * usbaudio_ctrl.h
 *
 * usbaudio ctrl.
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

#ifndef __USBAUDIO_CTRL_H__
#define __USBAUDIO_CTRL_H__

#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>

bool usbaudio_ctrl_controller_switch(struct usb_device *dev, u32 usb_id,
	struct usb_host_interface *ctrl_intf, int ctrlif);
int usbaudio_ctrl_disconnect(void);
void usbaudio_ctrl_set_chip(struct snd_usb_audio *chip);
void usbaudio_ctrl_wake_up(bool wake_up);

#endif /* __USBAUDIO_CTRL_H__ */
