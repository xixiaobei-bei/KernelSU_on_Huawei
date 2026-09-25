/*
 * hisi-usb-ldo.h
 *
 * utilityies for hifi-usb ldo
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HIFI_USB_LDO_H_
#define _HIFI_USB_LDO_H_

#include "hifi-usb.h"

#define MAX_IDO_CFG_BUFF_LEN    4
#define MAX_PHY_IDO_ADDR        0x7ff

extern void hifi_usb_phy_ldo_always_on(struct hifi_usb_proxy *hifi_usb);
extern void hifi_usb_phy_ldo_on(struct hifi_usb_proxy *hifi_usb);
extern void hifi_usb_phy_ldo_auto(struct hifi_usb_proxy *hifi_usb);
extern void hifi_usb_phy_ldo_force_auto(struct hifi_usb_proxy *hifi_usb);
extern void hifi_usb_phy_ldo_init(struct hifi_usb_proxy *hifiusb);

#endif /* _HIFI_USB_LDO_H_ */
