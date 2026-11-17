/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Header file for usb3_misctrl.c
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _DWC3_CHIP_COMMON__H
#define _DWC3_CHIP_COMMON__H

#include <linux/hisi/usb/hisi_usb_interface.h>

int dwc3_misc_ctrl_get(enum misc_ctrl_type type);
void dwc3_misc_ctrl_put(enum misc_ctrl_type type);

/* misc ctrl internal interface */
void init_misc_ctrl_addr(void __iomem *base);
void init_sc_misc_ctrl_addr(void __iomem *base);

void usb3_misc_reg_writel(u32 val, unsigned long int offset);
u32 usb3_misc_reg_readl(unsigned long int offset);
void usb3_misc_reg_setbit(u32 bit, unsigned long int offset);
void usb3_misc_reg_clrbit(u32 bit, unsigned long int offset);
void usb3_misc_reg_setvalue(u32 val, unsigned long int offset);
void usb3_misc_reg_clrvalue(u32 val, unsigned long int offset);

u32 usb3_sc_misc_reg_readl(unsigned long int offset);
void usb3_sc_misc_reg_writel(u32 val, unsigned long int offset);
void usb3_sc_misc_reg_setbit(u32 bit, unsigned long int offset);
void usb3_sc_misc_reg_clrbit(u32 bit, unsigned long int offset);
void usb3_sc_misc_reg_setvalue(u32 val, unsigned long int offset);
void usb3_sc_misc_reg_clrvalue(u32 val, unsigned long int offset);

int usb3_open_misc_ctrl_clk(void);
void usb3_close_misc_ctrl_clk(void);

#endif /* _DWC3_CHIP_COMMON__H */
