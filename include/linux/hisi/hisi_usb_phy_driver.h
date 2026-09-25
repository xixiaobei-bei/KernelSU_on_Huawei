/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: usb phy driver interface defination
 * Author: Hisilicon
 * Create: 2019-12-09
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __HISI_USB_PHY_DRIVER_H__
#define __HISI_USB_PHY_DRIVER_H__

int upc_hisi_bc_is_bcmode_on(void);
void upc_hisi_bc_dplus_pulldown(void);
void upc_hisi_bc_dplus_pullup(void);
void upc_hisi_bc_disable_vdp_src(void);
void upc_hisi_bc_enable_vdp_src(void);
enum hisi_charger_type upc_detect_charger_type(void);
void upc_set_eye_diagram_param(unsigned int snops_param,
	unsigned int hisi_param);
void upc_phy_reset(void);
void upc_phy_unreset(void);
bool upc_phy_driver_is_ready(void);

enum upc_burr_ops_type {
	UPC_BURR_PW_ON,
	UPC_BURR_PW_OFF
};
enum upc_burr_cfg_status_type {
	UPC_BURR_RESUME_RESTORE,
	UPC_BURR_NORMAL_CFG
};
void upc_remove_pw_burr_enable(enum upc_burr_ops_type enable,
	enum upc_burr_cfg_status_type cfg_mode);

#endif /* __HISI_USB_PHY_DRIVER_H__ */
