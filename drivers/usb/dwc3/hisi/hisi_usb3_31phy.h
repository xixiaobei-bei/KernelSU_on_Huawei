/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Header file for usb3_31phy.c
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _DWC3_CHIP_USB3_31PHY__H
#define _DWC3_CHIP_USB3_31PHY__H

#include <linux/hisi/usb/hisi_usb_interface.h>
#include <asm/types.h>

struct chip_usb_combophy {
	void (*reset_phy)(struct chip_usb_combophy *combophy);
	void (*reset_misc_ctrl)(struct chip_usb_combophy *combophy);
	void (*unreset_misc_ctrl)(struct chip_usb_combophy *combophy);
#ifdef COMBOPHY_FW_UPDATE
	void (*firmware_update_prepare)(struct chip_usb_combophy *combophy);
	void (*firmware_update)(struct chip_usb_combophy *combophy);
#endif
};

#define COMBOPHY_OPS_VOID(func) \
	static inline void combophy_##func(struct chip_usb_combophy *combophy) \
	{ \
		if (combophy && combophy->func) \
			combophy->func(combophy); \
	}

#define COMBOPHY_OPS_BOOL(func) \
	static inline int combophy_##func(struct chip_usb_combophy *combophy) \
	{ \
		if (combophy && combophy->func) \
			return combophy->func(combophy); \
		return false; \
	}

COMBOPHY_OPS_VOID(reset_phy);
COMBOPHY_OPS_VOID(reset_misc_ctrl);
COMBOPHY_OPS_VOID(unreset_misc_ctrl);
#ifdef COMBOPHY_FW_UPDATE
COMBOPHY_OPS_VOID(firmware_update_prepare);
COMBOPHY_OPS_VOID(firmware_update);
#endif

struct chip_usb_combophy *usb3_get_combophy_phandle(void);
int usb31phy_cr_write(u32 addr, u32 value);
u32 usb31phy_cr_read(u32 addr);

#endif /* _DWC3_CHIP_USB3_31PHY__H */
