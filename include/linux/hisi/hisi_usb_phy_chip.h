/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: usb phy chip basic interface defination
 * Author: Hisilicon
 * Create: 2019-12-09
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __HISI_USB_PHY_CHIP_H__
#define __HISI_USB_PHY_CHIP_H__

enum upc_chip_type {
	HISI_USB_PHY_CHIP = 0xAAAA,
	SNOPSYS_USB_PHY_CHIP = 0xFFFF,
	UNKNOWN_CHIP
};

void hi6502_readl_u32(unsigned int *val, unsigned int addr);
void hi6502_writel_u32(unsigned int val, unsigned int addr);
unsigned int hi6502_chip_type(void);
bool hi6502_is_ready(void);
bool hi6502_is_suspend(void);
void usb_misc_ulpi_reset(void);
void usb_misc_ulpi_unreset(void);
void upc_dump_reg(unsigned int start_addr, unsigned int len);
void upc_set_usb_ioc(unsigned int snopsys_data, unsigned int snopsys_stp,
	unsigned int hisi_data, unsigned int hisi_stp);
void upc_open(void);
void upc_close(void);

#endif /* __HISI_USB_PHY_CHIP_H__ */
