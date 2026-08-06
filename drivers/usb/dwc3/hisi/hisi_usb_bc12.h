/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: chip_usb_bc12.h for charger type check
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CHIP_USB_CHARGER_H_
#define _CHIP_USB_CHARGER_H_

#include <linux/hisi/usb/hisi_usb.h>
#include "dwc3-hisi.h"

/*
 * chip dwc3 bc registers
 */
#define BC_CTRL0		0x30  /* BC Control register 0 */
#define BC_CTRL1		0x34  /* BC Control register 1 */
#define BC_CTRL2		0x38  /* BC Control register 2 */
#define BC_STS0			0x3C  /* BC STS register 0 */

/* BC_CTRL0 */
# define BC_CTRL0_BC_IDPULLUP		(1 << 10)
# define BC_CTRL0_BC_SUSPEND_N		(1 << 9)
# define BC_CTRL0_BC_DMPULLDOWN		(1 << 8)
# define BC_CTRL0_BC_DPPULLDOWN		(1 << 7)
# define BC_CTRL0_BC_TXVALIDH		(1 << 6)
# define BC_CTRL0_BC_TXVALID		(1 << 5)
# define BC_CTRL0_BC_TERMSELECT		(1 << 4)
# define BC_CTRL0_BC_XCVRSELECT(x)	(((x) << 2) & (3 << 2))
# define BC_CTRL0_BC_OPMODE(x)		((x) & 3)

/* BC_CTRL1 */
# define BC_CTRL1_BC_MODE	1

/* BC_CTRL2 */
# define BC_CTRL2_BC_PHY_VDATDETENB	(1 << 4)
# define BC_CTRL2_BC_PHY_VDATARCENB	(1 << 3)
# define BC_CTRL2_BC_PHY_CHRGSEL		(1 << 2)
# define BC_CTRL2_BC_PHY_DCDENB		(1 << 1)
# define BC_CTRL2_BC_PHY_ACAENB		(1 << 0)

/* BC_STS0 */
# define BC_STS0_BC_LINESTATE(x)	(((x) << 9) & (3 << 9))
# define BC_STS0_BC_PHY_CHGDET		(1 << 8)
# define BC_STS0_BC_PHY_FSVMINUS	(1 << 7)
# define BC_STS0_BC_PHY_FSVPLUS		(1 << 6)
# define BC_STS0_BC_RID_GND		(1 << 5)
# define BC_STS0_BC_RID_FLOAT		(1 << 4)
# define BC_STS0_BC_RID_C		(1 << 3)
# define BC_STS0_BC_RID_B		(1 << 2)
# define BC_STS0_BC_RID_A		(1 << 1)
# define BC_STS0_BC_SESSVLD		(1 << 0)

/*
 * chip usb bc
 */

#define BC_AGAIN_DELAY_TIME_1 200
#define BC_AGAIN_DELAY_TIME_2 8000
#define BC_AGAIN_ONCE	1
#define BC_AGAIN_TWICE	2
void notify_charger_type(struct chip_dwc3_device *chip_dwc3);

/* bc interface */
#ifndef CONFIG_CHIP_USB_NEW_FRAME
void chip_bc_disable_vdp_src(struct chip_dwc3_device *chip_dwc3);
void chip_bc_enable_vdp_src(struct chip_dwc3_device *chip_dwc3);
void chip_bc_dplus_pulldown(struct chip_dwc3_device *chip_dwc);
void chip_bc_dplus_pullup(struct chip_dwc3_device *chip_dwc);
enum hisi_charger_type detect_charger_type(struct chip_dwc3_device *chip_dwc3);
#else
static inline void chip_bc_disable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
}
static inline void chip_bc_enable_vdp_src(struct chip_dwc3_device *chip_dwc3)
{
}
static inline void chip_bc_dplus_pulldown(struct chip_dwc3_device *chip_dwc)
{
}
static inline void chip_bc_dplus_pullup(struct chip_dwc3_device *chip_dwc)
{
}
static inline enum hisi_charger_type detect_charger_type(
		struct chip_dwc3_device *chip_dwc3)
{
	return CHARGER_TYPE_NONE;
}
#endif /* CONFIG_CHIP_USB_NEW_FRAME */

void schedule_bc_again(struct chip_dwc3_device *chip_dwc);
void cancel_bc_again(struct chip_dwc3_device *chip_dwc, int sync);
bool enumerate_allowed(const struct chip_dwc3_device *chip_dwc);
bool sleep_allowed(const struct chip_dwc3_device *chip_dwc);
bool bc_again_allowed(const struct chip_dwc3_device *chip_dwc);
int chip_usb_bc_init(struct chip_dwc3_device *chip_dwc);
void chip_usb_bc_exit(struct chip_dwc3_device *chip_dwc);
int chip_bc_is_bcmode_on(void);
#endif /* _CHIP_USB_CHARGER_H_ */
