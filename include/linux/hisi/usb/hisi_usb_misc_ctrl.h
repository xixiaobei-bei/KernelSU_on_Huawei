// SPDX-License-Identifier: GPL-2.0
/*
 * Head file of USB Misc Ctrl on HiSilicon platform
 *
 * Copyright (C) 2017-2018 Hilisicon Electronics Co., Ltd.
 *		http://www.huawei.com
 *
 */

#ifndef _HISI_USB_MISC_CTRL_H_
#define _HISI_USB_MISC_CTRL_H_

#ifdef CONFIG_HISI_USB_MISC_CTRL
extern int misc_ctrl_init(void);
extern void misc_ctrl_exit(void);
extern bool misc_ctrl_is_unreset(void);
extern bool misc_ctrl_is_ready(void);
#else
static inline int misc_ctrl_init(void) {return 0;}
static inline void misc_ctrl_exit(void) {}
static inline bool misc_ctrl_is_unreset(void) {return false;}
static inline bool misc_ctrl_is_ready(void) {return false;}
#endif /* _HISI_USB_MISC_CTRL_H_ */

#endif /* _HISI_USB_MISC_CTRL_H_ */
