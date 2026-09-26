/*
 * chip_usb_log.h
 *
 * Chip usb interface defination
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

#ifndef _HISI_USB_LOG_
#define _HISI_USB_LOG_
#include <linux/printk.h>

#ifdef CONFIG_CHIP_USB_LOG
#define hiusb_pr_info(fmt, arg...) pr_info("[HIUSB][I]%s:"fmt, __func__, ##arg)
#define hiusb_pr_err(fmt, arg...) pr_err("[HIUSB][E]%s:"fmt, __func__, ##arg)
#define hiusb_dev_info(dev, fmt, arg...) \
	dev_info(dev, "[HIUSB][I]%s:"fmt, __func__, ##arg)
#define hiusb_dev_err(dev, fmt, arg...) \
	dev_err(dev, "[HIUSB][E]%s:"fmt, __func__, ##arg)
#define hiusb_pr_info_ratelimited(fmt, arg...) \
	pr_info_ratelimited("[HIUSB][I]%s:"fmt, __func__, ##arg)
#define hiusb_pr_err_ratelimited(fmt, arg...) \
	pr_err_ratelimited("[HIUSB][E]%s:"fmt, __func__, ##arg)
#else
#define hiusb_pr_info(fmt, arg...)
#define hiusb_pr_err(fmt, arg...)
#define hiusb_dev_info(dev, fmt, arg...)
#define hiusb_dev_err(dev, fmt, arg...)
#define hiusb_pr_info_ratelimited(fmt, arg...)
#define hiusb_pr_err_ratelimited(fmt, arg...)
#endif /* CONFIG_CHIP_USB_LOG */

#endif /* _HISI_USB_LOG_ */
