// SPDX-License-Identifier: GPL-2.0
/*
 * Phy provider for USB 2.0 controller on HiSilicon May platform
 *
 * Copyright (C) 2017-2018 Hilisicon Electronics Co., Ltd.
 *		http://www.huawei.com
 *
 */

#ifndef _HISI_USB_PHY_H_
#define _HISI_USB_PHY_H_

#include <linux/phy/phy.h>
#include <linux/hisi/usb/hisi_usb.h>

struct hisi_usb2_phy {
	int (*shared_phy_init)(struct hisi_usb2_phy *usb2_phy);
	int (*shared_phy_exit)(struct hisi_usb2_phy *usb2_phy);
	enum hisi_charger_type (*detect_charger_type)(
			struct hisi_usb2_phy *usb2_phy);
	int (*set_dpdm_pulldown)(struct hisi_usb2_phy *usb2_phy, bool pulldown);
	int (*set_vdp_src)(struct hisi_usb2_phy *usb2_phy, bool enable);
};

#endif /* _HISI_USB_PHY_H_ */
