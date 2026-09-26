/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Header file for chip_usb.hw.c
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _CHIP_USB_HW_H_
#define _CHIP_USB_HW_H_

#include <linux/phy/phy.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_usb_phy.h>
#include "dwc3-hisi.h"

int chip_usb_get_hw_res(struct chip_dwc3_device *chip_usb, struct device *dev);
void chip_usb_put_hw_res(struct chip_dwc3_device *chip_usb);
int chip_usb3_phy_init(struct phy *phy,
		       enum tcpc_mux_ctrl_type mode_type,
		       enum typec_plug_orien_e typec_orien);
int chip_usb3_phy_exit(struct phy *phy);
int chip_usb2_phy_init(struct phy *usb2_phy, bool host_mode);
int chip_usb2_phy_exit(struct phy *usb2_phy);
enum hisi_charger_type chip_usb_detect_charger_type(
		struct chip_dwc3_device *chip_dwc3);
void chip_usb_dpdm_pulldown(struct chip_dwc3_device *chip_dwc3);
void chip_usb_dpdm_pullup(struct chip_dwc3_device *chip_dwc3);
void chip_usb_disable_vdp_src(struct chip_dwc3_device *chip_dwc3);
void chip_usb_enable_vdp_src(struct chip_dwc3_device *chip_dwc3);
int chip_usb_controller_init(struct chip_dwc3_device *chip_dwc3);
int chip_usb_controller_exit(struct chip_dwc3_device *chip_dwc3);
int chip_usb_register_hw_debugfs(struct chip_dwc3_device *chip_usb);
int chip_usb_controller_probe(struct chip_dwc3_device *chip_dwc3);
int chip_usb_controller_destroy(struct chip_dwc3_device *chip_dwc3);
void chip_usb3_phy_dump_info(struct chip_dwc3_device *chip_dwc);

#endif /* _CHIP_USB_HW_H_ */
