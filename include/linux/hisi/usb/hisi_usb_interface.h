/*
 * hisi_usb_interface.h
 *
 * Hisilicon usb interface defination
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef HISI_USB_INTERFACE_H
#define HISI_USB_INTERFACE_H
enum phy_change_type {
	PHY_MODE_CHANGE_BEGIN,
	PHY_MODE_CHANGE_END,
};

enum misc_ctrl_type {
	MICS_CTRL_USB = 0,
	MICS_CTRL_COMBOPHY = 1,
};

int chip_usb_combophy_notify(enum phy_change_type type);
int dwc3_misc_ctrl_get(enum misc_ctrl_type type);
void dwc3_misc_ctrl_put(enum misc_ctrl_type type);
#endif /* hisi_usb_interface.h */
