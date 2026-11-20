/*
 * chip_usb_misc_ctrl.h
 *
 * Hisilicon usb misc ctrl defination
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

#ifndef _HISI_USB_MISC_CTRL_H_
#define _HISI_USB_MISC_CTRL_H_

#ifdef CONFIG_CHIP_USB_MISC_CTRL
extern int misc_ctrl_init(void);
extern void misc_ctrl_exit(void);
extern bool misc_ctrl_is_unreset(void);
extern bool misc_ctrl_is_ready(void);
#else
static inline int misc_ctrl_init(void)
{
	return 0;
}
static inline void misc_ctrl_exit(void) {}
static inline bool misc_ctrl_is_unreset(void)
{
	return false;
}
static inline bool misc_ctrl_is_ready(void)
{
	return false;
}
#endif /* _HISI_USB_MISC_CTRL_H_ */

#endif /* _HISI_USB_MISC_CTRL_H_ */
